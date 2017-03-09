/** @file SOAView.h
 *
 * @author Manuel Schiller <Manuel.Schiller@cern.ch>
 * @date 2015-04-10
 */

#ifndef SOAVIEW_H
#define SOAVIEW_H

#include <limits>
#include <cassert>
#include <sstream>
#include <stdexcept>
#include <algorithm>
#include <functional>
#include <initializer_list>

#include "SOATypelist.h"
#include "SOATypelistUtils.h"
#include "SOAObjectProxy.h"
#include "SOADressedTuple.h"
#include "SOAIterator.h"
#include "SOAUtils.h"

/** @brief skin class for SOAView which does nothing and preserved the
 * raw proxy interface.
 *
 * @author Manuel Schiller <Manuel.Schiller@cern.ch>
 * @date 2015-04-21
 */
template <typename NAKEDPROXY>
class NullSkin : public NAKEDPROXY
{
    public:
        /// constructor - forward to underlying proxy
        template <typename... ARGS>
        NullSkin(ARGS&&... args)
            // ordinarily, we would like to have the following noexcept
            // specification here:
            //
            // noexcept(noexcept(NAKEDPROXY(std::forward<ARGS>(args)...)))
            //
            // however, gcc 4.9 and clang 3.5 insist that NAKEDPROXY's
            // constructor is protected and refuse the code in the exception
            // specification (despite the fact that it's perfectly legal to
            // call that very constructor in the initializer list below
            // because NullSkin<NAKEDPROXY> is a friend of NAKEDPROXY)
            : NAKEDPROXY(std::forward<ARGS>(args)...) { }

        /// assignment operator - forward to underlying proxy
        template <typename ARG>
        NullSkin<NAKEDPROXY>& operator=(const ARG& arg) noexcept(noexcept(
                    std::declval<NAKEDPROXY>().operator=(arg)))
        { NAKEDPROXY::operator=(arg); return *this; }

        /// move assignment operator - forward to underlying proxy
        template <typename ARG>
        NullSkin<NAKEDPROXY>& operator=(ARG&& arg) noexcept(noexcept(
                    std::declval<NAKEDPROXY>().operator=(
                        std::move(arg))))
        { NAKEDPROXY::operator=(std::move(arg)); return *this; }
};

// forward decl.
template < template <typename...> class CONTAINER,
         template <typename> class SKIN, typename... FIELDS>
class SOAContainer;

/** @brief SOA view for objects with given fields (SOA storage)
 *
 * @author Manuel Schiller <Manuel.Schiller@cern.ch>
 * @date 2015-04-10
 *
 * @tparam STORAGE      type to store the underlying ranges
 * @tparam SKIN         "skin" to dress the the interface of the object
 *                      proxying the content elemnts; this can be used to
 *                      augment the interface provided by the get<fieldtag>()
 *                      syntax with something more convenient; NullSkin leaves
 *                      the raw interface intact
 * @tparam FIELDS...    list of "field tags" describing names an types
 *                      of members
 *
 * This class represents an object view of a number of ranges of equal size
 * with the given list of fields.  Objects are not stored as such, but each of
 * object's fields is taken from the corresponding range, effectively creating
 * a structure-of-arrays (SOA) layout which is advantageous for vectorisation
 * of algorithms. To illustrate the SOA layout, first consider the normal
 * array-of-structures (AOS) layout:
 *
 * @code
 * class Point {
 *     private:
 *         float m_x;
 *         float m_y;
 *     public:
 *         Point(float x, float y) : m_x(x), m_y(y) { }
 *         float x() const noexcept { return m_x; }
 *         float y() const noexcept { return m_y; }
 *         void setX(float x) noexcept { m_x = x; }
 *         void setY(float y) noexcept { m_y = y; }
 *         // plus some routines that do more than just setting/getting members
 *         float r2() const noexcept
 *         { return m_x * m_x + m_y * m_y; }
 * };
 *
 * typedef std::vector<Point> AOSPoints;
 * typedef Point& AOSPoint;
 * @endcode
 *
 * The memory layout in the example above will be x of element 0, y of element
 * 0, x of element 1, y of element 1, x of element 2, and so on.
 *
 * For the equivalent example in SOA layout, you'd have to do:
 *
 * @code
 * #include "SOAView.h"
 *
 * // first declare member "tags" which describe the members of the notional
 * // struct (which will never exist in memory - SOA layout!)
 *  namespace PointFields {
 *     using namespace SOATypelist;
 *     // since we can have more than one member of the same type in our
 *     // SOA object, we have to do some typedef gymnastics so the compiler
 *     // can tell them apart
 *     typedef struct : public wrap_type<float> { } x;
 *     typedef struct : public wrap_type<float> { } y;
 * };
 *
 * // define the "skin", i.e. the outer guise that the naked members "wear"
 * // to make interaction with the class nice
 * template <typename NAKEDPROXY>
 * class SOAPoint : public NAKEDPROXY {
 *     public:
 *         /// forward constructor to NAKEDPROXY's constructor
 *         template <typename... ARGS>
 *         SOAPoint(ARGS&&... args) :
 *             NAKEDPROXY(std::forward<ARGS>(args)...) { }
 *
 *         /// assignment operator - forward to underlying proxy
 *         template <typename ARG>
 *         SOAPoint<NAKEDPROXY>& operator=(const ARG& arg)
 *         { NAKEDPROXY::operator=(arg); return *this; }
 *
 *         /// move assignment operator - forward to underlying proxy
 *         template <typename ARG>
 *         SOAPoint<NAKEDPROXY>& operator=(ARG&& arg)
 *         { NAKEDPROXY::operator=(std::move(arg)); return *this; }
 *
 *         float x() const noexcept
 *         { return this-> template get<PointFields::x>(); }
 *         float y() const noexcept
 *         { return this-> template get<PointFields::y>(); }
 *         void setX(float x) noexcept
 *         { this-> template get<PointFields::x>() = x; }
 *         void setY(float y) noexcept
 *         { this-> template get<PointFields::y>() = y; }
 *
 *         // again, something beyond plain setters/getters
 *         float r2() const noexcept { return x() * x() + y() * y(); }
 * };
 *
 * // define the SOA container type
 * typedef SOAView<
 *         std::vector, // underlying type for each field
 *         SOAPoint,    // skin to "dress" the tuple of fields with
 *         // one or more wrapped types which each tag a member/field
 *         PointFields::x, PointFields::y> SOAPoints;
 * // define the SOAPoint itself
 * typedef typename SOAPoints::proxy SOAPoint;
 * @endcode
 *
 * The code is very similar to the AOS layout example above, but the memory
 * layout is very different. Internally, the container has two std::vectors,
 * one which holds all the x "members" of the point structure, and one which
 * holds all the "y" members.
 *
 * Despite all the apparent complexity in defining this container, the use of
 * the contained points is practically the same in both cases. For example,
 * consider a piece of code the "normalises" all points to have unit length:
 *
 * @code
 * // normalise to unit length for the old-style AOSPoint
 * for (AOSPoint p: points) {
 *     auto ir = 1 / std::sqrt(p.r2());
 *     p.setX(p.x() * ir), p.setY(p.y() * ir);
 * }
 * @endcode
 *
 * The only change required is that a different data type is used in
 * conjunction with the SOAPoint implementation from above:
 *
 * @code
 * // normalise to unit length
 * for (SOAPoint p: points) {
 *     auto ir = 1 / std::sqrt(p.r2());
 *     p.setX(p.x() * ir), p.setY(p.y() * ir);
 * }
 * @endcode
 *
 * It is important to realise that there's nothing like the struct Point in the
 * AOS example above - the notion of a point very clearly exists, but the
 * SOAView will not create such an object at any point in time. Instead,
 * the container provides an interface to access fields with the get<field_tag>
 * syntax, or via tuples (conversion to or assignment from tuples is
 * implemented), if simultaneous access to all fields is desired.
 *
 * Apart from these points (which are dictated by the SOA layout and efficiency
 * considerations), the class tries to follow the example of the interface of
 * std::vector as closely as possible.
 *
 * When using the SOAView class, one must distinguish between elements
 * (and references to elements) which are stored outside the container and
 * those that are stored inside the container:
 *
 * - When stored inside the container, the element itself does not exist as
 *   such because of the SOA storage constraint; (const) references and
 *   pointers are implemented with instances of SKINned SOAObjectProxy and
 *   SOA(Const)Ptr. On the outside, these classes look and feel like
 *   references and pointers, but set up memory access to members/fields such
 *   that the SOA memory layout is preserved.
 * - When a single element is stored outside of the container (a temporary),
 *   the SOA layout doesn't make sense (it's just a single element, after
 *   all), and it is stored as a SKINned std::tuple with the right members.
 *   The value_type, value_reference and const_value_reference typedefs name
 *   the type of these objects.
 *
 * This distinction becomes important when using or implementing generic
 * algorithms (like types in the functor passed to std::sort) which sometimes
 * create a temporary holding a single element outside the container. For
 * illustration, here's how one would sort by increasing y in the SOAPoint
 * example from above:
 *
 * @code
 * SOAPoints& c = get_points_from_elsewhere();
 * std::sort(c.begin(), c.end(),
 *     [] (decltype(c)::value_const_reference a,
 *         decltype(c)::value_const_reference b)
 *     { return a.y() < b.y(); });
 * @endcode
 */
template <class STORAGE,
    template <typename> class SKIN, typename... FIELDS>
class SOAView {
    private:
        template <template <typename> class PRED, typename... ARGS>
        using ANY = SOAUtils::ANY<PRED, ARGS...>;
        template <template <typename> class PRED, typename... ARGS>
        using ALL = SOAUtils::ALL<PRED, ARGS...>;
        /// little helper to find the type contained in a range
        template <typename RANGE>
        struct contained_type {
            typedef decltype(*std::begin(std::declval<RANGE>())) type;
        };
        /// hide verification of FIELDS inside struct or doxygen gets confused
        struct fields_verifier {
            // storing objects without state doesn't make sense
            static_assert(1 <= sizeof...(FIELDS),
                "need to supply at least one field");
            /// little helper to verify the FIELDS template parameter
            template <typename T>
            struct is_pod_or_wrapped : std::integral_constant<bool,
                std::is_pod<T>::value ||
                SOATypelist::is_wrapped<T>::value> {};
            // and check that all fields are either pod or wrapped
            static_assert(ALL<is_pod_or_wrapped, FIELDS...>::value,
                "Fields should be either plain old data (POD) or "
                "wrapped types.");

            // make sure storage size agrees with number of fields
            static_assert(std::tuple_size<STORAGE>::value ==
                    sizeof...(FIELDS),
                    "Number of fields does not match storage.");
            /// little helper: check if element in a tuple F matches field
            template <size_t N, class T, typename FIELD>
            struct verify_storage_element : public std::integral_constant<bool,
                std::is_same<typename contained_type<
                    typename std::tuple_element<N, T>::type>::type,
                    SOATypelist::unwrap_t<FIELD> >::value ||
                std::is_same<typename std::remove_cv<typename contained_type<
                    typename std::tuple_element<N, T>::type>::type>::type,
                    SOATypelist::unwrap_t<FIELD> >::value ||
                std::is_same<typename std::remove_cv<
                    typename std::remove_reference<typename contained_type<
                    typename std::tuple_element<N, T>::type>::type>::type>::type,
                    SOATypelist::unwrap_t<FIELD> >::value> {};

            /// little helper verifying the storage matches the fields
            template <size_t N, class T, typename... ARGS>
            struct verify_storage;
            /// specialisation for > 1 field
            template <size_t N, class T, typename HEAD, typename... TAIL>
            struct verify_storage<N, T, HEAD, TAIL...> : public
                std::integral_constant<bool,
                    verify_storage_element<N, T, HEAD>::value &&
                    verify_storage<N + 1, T, TAIL...>::value> { };
            /// specialisation for one field
            template <size_t N, class T, typename HEAD>
            struct verify_storage<N, T, HEAD> : public
                std::integral_constant<bool,
                    verify_storage_element<N, T, HEAD>::value> { };
            /// specialisation for empty tuples
            template <size_t N, typename... ARGS>
            struct verify_storage<N, std::tuple<>, ARGS...> :
                public std::false_type { };
            // make sure the storage matches the fields provided
            static_assert(verify_storage<0, STORAGE, FIELDS...>::value,
                    "Type of provided storage must match fields.");
        };

        /// work out if what is contained in a range is constant
        template <typename RANGE>
        struct is_contained_constant : std::integral_constant<bool,
            std::is_const<RANGE>::value || std::is_const<
                typename contained_type<RANGE>::type>::value> { };

        /// is any field a constant range?
        template <typename... ARGS>
        struct _is_any_field_constant : ANY<is_contained_constant, ARGS...> { };

        /// helper for _is_any_field_constant: extract parameter pack
        template <template <typename...> class T, typename... ARGS>
        constexpr static bool is_any_field_constant(const T<ARGS...>*) noexcept
        { return _is_any_field_constant<ARGS...>::value; }

        /// record if the SOAView should be a const one
        enum {
            is_constant = SOAView<STORAGE, SKIN, FIELDS...
                >::is_any_field_constant(static_cast<const STORAGE*>(nullptr))
        };

    public:
        /// type to represent sizes and indices
        typedef std::size_t size_type;
        /// type to represent differences of indices
        typedef std::ptrdiff_t difference_type;
        /// type to represent container itself
        typedef SOAView<STORAGE, SKIN, FIELDS...> self_type;
        /// typedef holding a typelist with the given fields
        typedef SOATypelist::typelist<FIELDS...> fields_typelist;
        /// type of the storage backend
        typedef STORAGE SOAStorage;

        /// convenience function to return member number given member tag type
        template <typename MEMBER>
        static constexpr size_type memberno() noexcept
        { return fields_typelist::template find<MEMBER>(); }

    protected:
        /// implementation details
        struct impl_detail {
            /// little helper for assign(count, val)
            struct assignHelper {
                size_type m_cnt;
                template <typename T, typename V>
                void operator()(std::tuple<T&, const V&> t) const noexcept(noexcept(
                        std::get<0>(t).assign(m_cnt, std::get<1>(t))))
                { std::get<0>(t).assign(m_cnt, std::get<1>(t)); }
            };
            template <typename IT, size_type IDX>
            struct tuple_element_iterator {
                IT m_it;
                tuple_element_iterator& operator++() noexcept(
                        noexcept(++m_it))
                { ++m_it; return *this; }
                auto operator*() const noexcept(
                        noexcept(std::get<IDX>(*m_it))) -> decltype(
                        std::get<IDX>(*m_it))
                { return std::get<IDX>(*m_it); }
            };
            template <typename IT, size_type... IDXS>
            static std::tuple<tuple_element_iterator<IT, IDXS>...>
            make_tuple_element_iterators(IT it, std::index_sequence<IDXS...>)
            {
                return std::tuple<tuple_element_iterator<IT, IDXS>...>(
                        tuple_element_iterator<IT, IDXS...>{it});
            }
            template <size_type N, typename IT>
            static auto make_tuple_element_iterators(IT it) -> decltype(
                    make_tuple_element_iterators(it,
                        std::make_index_sequence<N>()))
            {
                return make_tuple_element_iterators(it,
                        std::make_index_sequence<N>());
            }
            /// little helper for assign(first, last)
            struct assignHelper2 {
                template <typename R, typename IT>
                void operator()(std::tuple<R&, IT>& t) const noexcept(noexcept(
                            *std::get<0>(t).begin() = *std::get<1>(t)))
                {
                    auto& it = std::get<1>(t), itend = std::get<2>(t);
                    auto& jt = std::get<0>(t).begin();
                    while (itend != it) {
                        *jt = *it;
                        ++jt, ++it;
                    }
                }
            };
        };

    protected:
        /// storage backend
        SOAStorage m_storage;

        /// (naked) tuple type used as values
        typedef typename SOATypelist::to_tuple<
            fields_typelist>::value_tuple naked_value_tuple_type;
        /// (naked) tuple type used as reference
        typedef typename SOATypelist::to_tuple<
            fields_typelist>::reference_tuple naked_reference_tuple_type;
        /// (naked) tuple type used as const reference
        typedef typename SOATypelist::to_tuple<
            fields_typelist>::const_reference_tuple naked_const_reference_tuple_type;

        SOAView() {}

    public:
        /// (notion of) type of the contained objects
        typedef SKIN<DressedTuple<naked_value_tuple_type, self_type> >
            value_type;
        /// (notion of) reference to value_type (outside container)
        typedef SKIN<DressedTuple<naked_reference_tuple_type, self_type> >
            value_reference;
        /// (notion of) const reference to value_type (outside container)
        typedef SKIN<DressedTuple<naked_const_reference_tuple_type,
                self_type> > value_const_reference;

    public:
        /// naked proxy type (to be given a "skin" later)
        typedef SOAObjectProxy<self_type> naked_proxy;
        friend naked_proxy;
        /// corresponding SOAContainers are friends
        template < template <typename...> class CONTAINER,
                 template <typename> class SKIN2, typename... FIELDS2>
        friend class SOAContainer;
        /// type of proxy
        typedef SKIN<naked_proxy> proxy;
        friend proxy;
        /// pointer to contained objects
        typedef SOAIterator<proxy> pointer;
        friend pointer;
        /// iterator type
        typedef pointer iterator;
        /// reference to contained objects
        typedef proxy reference;
        /// reference to contained objects
        typedef const reference const_reference;
        /// const pointer to contained objects
        typedef SOAConstIterator<proxy> const_pointer;
        friend const_pointer;
        /// const iterator type
        typedef const_pointer const_iterator;
        /// reverse iterator type
        typedef std::reverse_iterator<iterator> reverse_iterator;
        /// const reverse iterator type
        typedef std::reverse_iterator<const_iterator> const_reverse_iterator;

        /// constructor from the underlying storage
        SOAView(const SOAStorage& other) :
            m_storage(other) { }
        /// constructor from the underlying storage
        SOAView(SOAStorage&& other) :
            m_storage(std::move(other)) { }
        /// constructor from a list of ranges
        template <typename... RANGES, typename std::enable_if<sizeof...(RANGES) == sizeof...(FIELDS), int>::type = 0>
        SOAView(RANGES&&... ranges) :
            m_storage(std::forward<RANGES>(ranges)...) { }
        /// copy constructor
        SOAView(const self_type& other) = default;
        /// move constructor
        SOAView(self_type&& other) = default;

        /// assignment from other SOAView
        self_type& operator=(const self_type& other) = default;
        /// move-assignment from other SOAView
        self_type& operator=(self_type&& other) = default;

        /// return if the container is empty
        bool empty() const noexcept(noexcept(std::get<0>(m_storage).empty()))
        { return std::get<0>(m_storage).empty(); }
        /// return the size of the container
        size_type size() const noexcept(noexcept(
                    std::get<0>(m_storage).size()))
        { return std::get<0>(m_storage).size(); }

        /// access specified element
        typename std::enable_if<!is_constant, reference>::type
        operator[](size_type idx) noexcept
        { return { &m_storage, idx }; }
        /// access specified element (read access only)
        const_reference operator[](size_type idx) const noexcept
        { return { &const_cast<SOAStorage&>(m_storage), idx }; }
        /// access specified element with out of bounds checking
        typename std::enable_if<!is_constant, reference>::type
        at (size_type idx)
        {
            if (idx < size()) return operator[](idx);
            else throw std::out_of_range("out of bounds");
        }
        /// access specified element with out of bounds checking (read-only)
        const_reference at(size_type idx) const
        {
            if (idx < size()) return operator[](idx);
            else throw std::out_of_range("out of bounds");
        }

        /// access first element (non-empty container)
        typename std::enable_if<!is_constant, reference>::type
        front() noexcept { return operator[](0); }
        /// access first element (non-empty container, read-only)
        const_reference front() const noexcept { return operator[](0); }
        /// access last element (non-empty container)
        typename std::enable_if<!is_constant, reference>::type
        back() noexcept(noexcept(
                    std::declval<self_type>().size()))
        { return operator[](size() - 1); }
        /// access last element (non-empty container, read-only)
        const_reference back() const noexcept(noexcept(
                    std::declval<self_type>().size()))
        { return operator[](size() - 1); }

        /// iterator pointing to first element
        typename std::enable_if<!is_constant, iterator>::type
        begin() noexcept { return { &m_storage, 0 }; }
        /// iterator pointing one element behind the last element
        typename std::enable_if<!is_constant, iterator>::type
        end() noexcept { return { &m_storage, size() }; }

        /// const iterator pointing to first element
        const_iterator begin() const noexcept
        { return { const_cast<SOAStorage*>(&m_storage), 0 }; }
        /// const iterator pointing one element behind the last element
        const_iterator end() const noexcept
        { return { const_cast<SOAStorage*>(&m_storage), size() }; }

        /// const iterator pointing to first element
        const_iterator cbegin() const noexcept { return begin(); }
        /// const iterator pointing one element behind the last element
        const_iterator cend() const noexcept { return end(); }

        /// get begin iterator of storage vector for member MEMBERNO
        template <size_type MEMBERNO>
        typename std::enable_if<!is_constant, decltype(
                std::get<MEMBERNO>(std::declval<SOAStorage>()).begin())>::type
        begin() noexcept
        { return std::get<MEMBERNO>(m_storage).begin(); }
        /// get end iterator of storage vector for member MEMBERNO
        template <size_type MEMBERNO>
        typename std::enable_if<!is_constant, decltype(
                std::get<MEMBERNO>(std::declval<SOAStorage>()).end())>::type
        end() noexcept
        { return std::get<MEMBERNO>(m_storage).end(); }

        /// get begin iterator of storage vector for member with tag MEMBER
        template <typename MEMBER>
        typename std::enable_if<!is_constant, decltype(
                std::get<memberno<MEMBER>()>(std::declval<SOAStorage>()).begin())>::type
        begin() noexcept
        { return std::get<memberno<MEMBER>()>(m_storage).begin(); }
        /// get end iterator of storage vector for member with tag MEMBER
        template <typename MEMBER>
        typename std::enable_if<!is_constant, decltype(
                std::get<memberno<MEMBER>()>(std::declval<SOAStorage>()).end())>::type
        end() noexcept
        { return std::get<memberno<MEMBER>()>(m_storage).end(); }

        /// get begin iterator of storage vector for member MEMBERNO
        template <size_type MEMBERNO>
        auto begin() const noexcept -> decltype(
                std::get<MEMBERNO>(m_storage).begin())
        { return std::get<MEMBERNO>(m_storage).begin(); }
        /// get end iterator of storage vector for member MEMBERNO
        template <size_type MEMBERNO>
        auto end() const noexcept -> decltype(
                std::get<MEMBERNO>(m_storage).end())
        { return std::get<MEMBERNO>(m_storage).end(); }

        /// get begin iterator of storage vector for member with tag MEMBER
        template <typename MEMBER>
        auto begin() const noexcept -> decltype(
                std::get<memberno<MEMBER>()>(m_storage).begin())
        { return std::get<memberno<MEMBER>()>(m_storage).begin(); }
        /// get end iterator of storage vector for member with tag MEMBER
        template <typename MEMBER>
        auto end() const noexcept -> decltype(
                std::get<memberno<MEMBER>()>(m_storage).end())
        { return std::get<memberno<MEMBER>()>(m_storage).end(); }

        /// get begin iterator of storage vector for member MEMBERNO
        template <size_type MEMBERNO>
        auto cbegin() const noexcept -> decltype(
                std::get<MEMBERNO>(m_storage).cbegin())
        { return std::get<MEMBERNO>(m_storage).cbegin(); }
        /// get end iterator of storage vector for member MEMBERNO
        template <size_type MEMBERNO>
        auto cend() const noexcept -> decltype(
                std::get<MEMBERNO>(m_storage).cend())
        { return std::get<MEMBERNO>(m_storage).cend(); }

        /// get begin iterator of storage vector for member with tag MEMBER
        template <typename MEMBER>
        auto cbegin() const noexcept -> decltype(
                std::get<memberno<MEMBER>()>(m_storage).cbegin())
        { return std::get<memberno<MEMBER>()>(m_storage).cbegin(); }
        /// get end iterator of storage vector for member with tag MEMBER
        template <typename MEMBER>
        auto cend() const noexcept -> decltype(
                std::get<memberno<MEMBER>()>(m_storage).cend())
        { return std::get<memberno<MEMBER>()>(m_storage).cend(); }


        /// iterator pointing to first element
        typename std::enable_if<!is_constant, reverse_iterator>::type
        rbegin() noexcept { return reverse_iterator(end()); }
        /// iterator pointing one element behind the last element
        typename std::enable_if<!is_constant, reverse_iterator>::type
        rend() noexcept { return reverse_iterator(begin()); }

        /// const iterator pointing to first element
        const_reverse_iterator rbegin() const noexcept
        { return const_reverse_iterator(end()); }
        /// const iterator pointing one element behind the last element
        const_reverse_iterator rend() const noexcept
        { return const_reverse_iterator(begin()); }

        /// const iterator pointing to first element
        const_reverse_iterator crbegin() const noexcept { return rbegin(); }
        /// const iterator pointing one element behind the last element
        const_reverse_iterator crend() const noexcept { return rend(); }

        /// get begin iterator of storage vector for member MEMBERNO
        template <size_type MEMBERNO>
        typename std::enable_if<!is_constant, decltype(
                std::get<MEMBERNO>(std::declval<SOAStorage>()).rbegin())>::type
        rbegin() noexcept
        { return std::get<MEMBERNO>(m_storage).rbegin(); }
        /// get end iterator of storage vector for member MEMBERNO
        template <size_type MEMBERNO>
        typename std::enable_if<!is_constant, decltype(
                std::get<MEMBERNO>(std::declval<SOAStorage>()).rend())>::type
        rend() noexcept
        { return std::get<MEMBERNO>(m_storage).rend(); }

        /// get begin iterator of storage vector for member with tag MEMBER
        template <typename MEMBER>
        typename std::enable_if<!is_constant, decltype(
                std::get<memberno<MEMBER>()>(std::declval<SOAStorage>()).rbegin())>::type
        rbegin() noexcept
        { return std::get<memberno<MEMBER>()>(m_storage).rbegin(); }
        /// get end iterator of storage vector for member with tag MEMBER
        template <typename MEMBER>
        typename std::enable_if<!is_constant, decltype(
                std::get<memberno<MEMBER>()>(std::declval<SOAStorage>()).rend())>::type
        rend() noexcept
        { return std::get<memberno<MEMBER>()>(m_storage).rend(); }

        /// get begin iterator of storage vector for member MEMBERNO
        template <size_type MEMBERNO>
        auto rbegin() const noexcept -> decltype(
                std::get<MEMBERNO>(m_storage).rbegin())
        { return std::get<MEMBERNO>(m_storage).rbegin(); }
        /// get end iterator of storage vector for member MEMBERNO
        template <size_type MEMBERNO>
        auto rend() const noexcept -> decltype(
                std::get<MEMBERNO>(m_storage).rend())
        { return std::get<MEMBERNO>(m_storage).rend(); }

        /// get begin iterator of storage vector for member with tag MEMBER
        template <typename MEMBER>
        auto rbegin() const noexcept -> decltype(
                std::get<memberno<MEMBER>()>(m_storage).rbegin())
        { return std::get<memberno<MEMBER>()>(m_storage).rbegin(); }
        /// get end iterator of storage vector for member with tag MEMBER
        template <typename MEMBER>
        auto rend() const noexcept -> decltype(
                std::get<memberno<MEMBER>()>(m_storage).rend())
        { return std::get<memberno<MEMBER>()>(m_storage).rend(); }

        /// get begin iterator of storage vector for member MEMBERNO
        template <size_type MEMBERNO>
        auto crbegin() const noexcept -> decltype(
                std::get<MEMBERNO>(m_storage).crbegin())
        { return std::get<MEMBERNO>(m_storage).crbegin(); }
        /// get end iterator of storage vector for member MEMBERNO
        template <size_type MEMBERNO>
        auto crend() const noexcept -> decltype(
                std::get<MEMBERNO>(m_storage).crend())
        { return std::get<MEMBERNO>(m_storage).crend(); }

        /// get begin iterator of storage vector for member with tag MEMBER
        template <typename MEMBER>
        auto crbegin() const noexcept -> decltype(
                std::get<memberno<MEMBER>()>(m_storage).crbegin())
        { return std::get<memberno<MEMBER>()>(m_storage).crbegin(); }
        /// get end iterator of storage vector for member with tag MEMBER
        template <typename MEMBER>
        auto crend() const noexcept -> decltype(
                std::get<memberno<MEMBER>()>(m_storage).crend())
        { return std::get<memberno<MEMBER>()>(m_storage).crend(); }

        /// assign the vector to contain count copies of val
        void assign(size_type count, const value_type& val)
        {
            if (size() >= count) {
                std::stringstream str;
                str << "In " << __func__ << " (" << __FILE__ << ", line " <<
                    __LINE__ << "): count must not exceed length of range.";
                throw std::out_of_range(str.str());
            }
            SOAUtils::map(typename impl_detail::assignHelper{count},
                    SOAUtils::zip(m_storage, val),
                    std::make_index_sequence<sizeof...(FIELDS)>());
        }

        /// assign the vector from a range of elements in another container
        template <typename IT>
        void assign(IT first, IT last)
        {
            if (size() >= std::distance(first, last)) {
                std::stringstream str;
                str << "In " << __func__ << " (" << __FILE__ << ", line " <<
                    __LINE__ << "): supplied range too large.";
                throw std::out_of_range(str.str());
            }
            SOAUtils::map(typename impl_detail::assignHelper2(),
                    SOAUtils::zip(m_storage,
                        impl_detail::template make_tuple_element_iterators<
                            sizeof...(FIELDS)>(first),
                        impl_detail::template make_tuple_element_iterators<
                            sizeof...(FIELDS)>(last)),
                    std::make_index_sequence<sizeof...(FIELDS)>());
        }

        /// swap contents of two containers
        void swap(self_type& other) noexcept(
                noexcept(std::swap(m_storage, other.m_storage)))
        { std::swap(m_storage, other.m_storage); }
};

/** @contstruct a SOAView from a skin and a bunch of ranges
 *
 * @tparam SKIN         type of skin class to use
 * @tparam FIELDS       types of fields
 * @tparam RANGES       types of the ranges supplied
 *
 * @param ranges        ranges from which to construct a SOAView
 *
 * @returns a SOAView of the ranges given
 *
 * @code
 * std::vector<float> vx, vy;
 * // fill vx, vy somehow - same number of elements
 * typedef struct : SOATypelist::wrap_type<float> {} field_x;
 * typedef struct : SOATypelist::wrap_type<float> {} field_y;
 * template <typename NAKEDPROXY>
 * class SOAPoint : public NAKEDPROXY {
 *     public:
 *         template <typename... ARGS>
 *         SOAPoint(ARGS&&... args) :
 *             NAKEDPROXY(std::forward<ARGS>(args)...) { }
 *         template <typename ARG>
 *         SOAPoint<NAKEDPROXY>& operator=(const ARG& arg)
 *         { NAKEDPROXY::operator=(arg); return *this; }
 *         template <typename ARG>
 *         SOAPoint<NAKEDPROXY>& operator=(ARG&& arg)
 *         { NAKEDPROXY::operator=(std::move(arg)); return *this; }
 *
 *         float x() const noexcept
 *         { return this-> template get<field_x>(); }
 *         float y() const noexcept
 *         { return this-> template get<field_y>(); }
 *         float& x() noexcept
 *         { return this-> template get<field_x>(); }
 *         float& y() noexcept
 *         { return this-> template get<field_y>(); }
 *         float r2() const noexcept { return x() * x() + y() * y(); }
 * };
 * // construct a SOAView from vx, vy
 * auto view = make_soaview<SOAPoint, field_x, field_y>(vx, vy);
 * const float angle = 42.f / 180.f * M_PI;
 * const auto s = std::sin(angle), c = std::cos(angle);
 * for (auto p: view) {
 *     if (p.r2() > 1) continue;
 *     // rotate points within the unit circle by given angle
 *     std::tie(p.x(), p.y()) = std::make_pair(
 *         c * p.x() + s * p.y(), -s * p.x() + c * p.y());
 * }
 * @endcode
 */
template <template <typename> class SKIN,
         typename... FIELDS, typename... RANGES>
SOAView<std::tuple<RANGES...>, SKIN, FIELDS...>
make_soaview(RANGES&&... ranges)
{
    return SOAView<std::tuple<RANGES...>, SKIN, FIELDS...>
        (std::forward<RANGES>(ranges)...);
}

namespace std {
    /// specialise std::swap
    template <typename STORAGE,
             template <typename> class SKIN,
             typename... FIELDS>
    void swap(const SOAView<STORAGE, SKIN, FIELDS...>& a,
            const SOAView<STORAGE, SKIN, FIELDS...>& b) noexcept(
                noexcept(a.swap(b)))
    { a.swap(b); }
}

/// more SOAView implementation details
namespace SOAViewImpl {
    /// helper class to compare SOAViews (field by field)
    template <template <typename> class COMP, std::size_t N> class compare {
        private:
            /// compare field N - 1 element by element
            template <typename T>
            static bool doit(const T& a, const T& b) noexcept(noexcept(
                        COMP<decltype(a.front().template get<N - 1>())>()(
                                a.front().template get<N - 1>(),
                                b.front().template get<N - 1>())))
            {
                auto last = std::min(a.size(), b.size());
                for (auto it = a.template cbegin<N - 1>(),
                        jt = a.template cbegin<N - 1>() + last,
                        kt = b.template cbegin<N - 1>();
                        jt != it; ++it, ++kt) {
                    if (!COMP<decltype(*it)>()(*it, *kt)) return false;
                }
                return true;
            }
        public:
            /// trigger comparison of fields 0, ..., N - 1
            template <typename T>
            bool operator()(const T& a, const T& b) const noexcept(noexcept(
                        compare<COMP, N - 1>()(a, b)) && noexcept(doit(a, b)))
            { return compare<COMP, N - 1>()(a, b) && doit(a, b); }
    };

    /// helper class to compare SOAViews, specialised N = 1
    template <template <typename> class COMP> class compare<COMP, 1> {
        private:
            /// compare field 0 element by element
            template <typename T>
            static bool doit(const T& a, const T& b) noexcept(noexcept(
                        COMP<decltype(a.front().template get<0>())>()(
                                a.front().template get<0>(),
                                b.front().template get<0>())))
            {
                auto last = std::min(a.size(), b.size());
                for (auto it = a.template cbegin<0>(),
                        jt = a.template cbegin<0>() + last,
                        kt = b.template cbegin<0>();
                        jt != it; ++it, ++kt) {
                    if (!COMP<decltype(*it)>()(*it, *kt)) return false;
                }
                return true;
            }
        public:
            /// trigger comparison of field 0
            template <typename T>
            bool operator()(const T& a, const T& b) const
                noexcept(noexcept(doit(a, b)))
            { return doit(a, b); }
    };
}

/// compare two SOAViews for equality
template <typename STORAGE,
    template <typename> class SKIN, typename... FIELDS>
bool operator==(const SOAView<STORAGE, SKIN, FIELDS...>& a,
        const SOAView<STORAGE, SKIN, FIELDS...>& b) noexcept(
            noexcept(a.size()) && noexcept(
                SOAViewImpl::compare<std::equal_to,
                SOAView<STORAGE, SKIN, FIELDS...
                >::fields_typelist::size()>()(a, b)))
{
    if (a.size() != b.size()) return false;
    // compare one field at a time
    return SOAViewImpl::compare<std::equal_to,
           SOAView<STORAGE, SKIN, FIELDS...
               >::fields_typelist::size()>()(a, b);
}

/// compare two SOAViews for inequality
template <typename STORAGE,
    template <typename> class SKIN, typename... FIELDS>
bool operator!=(const SOAView<STORAGE, SKIN, FIELDS...>& a,
        const SOAView<STORAGE, SKIN, FIELDS...>& b) noexcept(
            noexcept(a.size()) && noexcept(
                SOAViewImpl::compare<std::not_equal_to,
                SOAView<STORAGE, SKIN, FIELDS...
                >::fields_typelist::size()>()(a, b)))
{
    if (a.size() != b.size()) return true;
    // compare one field at a time
    return SOAViewImpl::compare<std::not_equal_to,
           SOAView<STORAGE, SKIN, FIELDS...
               >::fields_typelist::size()>()(a, b);
}

/// compare two SOAViews lexicographically using <
template <typename STORAGE,
    template <typename> class SKIN, typename... FIELDS>
bool operator<(const SOAView<STORAGE, SKIN, FIELDS...>& a,
        const SOAView<STORAGE, SKIN, FIELDS...>& b) noexcept(
            noexcept(std::lexicographical_compare(a.cbegin(), a.cend(),
                    b.cbegin(), b.cend(),
                    std::less<decltype(a.front())>())))
{
    return std::lexicographical_compare(a.cbegin(), a.cend(),
            b.cbegin(), b.cend(), std::less<decltype(a.front())>());
}

/// compare two SOAViews lexicographically using >
template <typename STORAGE,
    template <typename> class SKIN, typename... FIELDS>
bool operator>(const SOAView<STORAGE, SKIN, FIELDS...>& a,
        const SOAView<STORAGE, SKIN, FIELDS...>& b) noexcept(
            noexcept(b < a))
{ return b < a; }

/// compare two SOAViews lexicographically using <=
template <typename STORAGE,
    template <typename> class SKIN, typename... FIELDS>
bool operator<=(const SOAView<STORAGE, SKIN, FIELDS...>& a,
        const SOAView<STORAGE, SKIN, FIELDS...>& b) noexcept(
            noexcept(!(a > b)))
{ return !(a > b); }

/// compare two SOAViews lexicographically using >=
template <typename STORAGE,
    template <typename> class SKIN, typename... FIELDS>
bool operator>=(const SOAView<STORAGE, SKIN, FIELDS...>& a,
        const SOAView<STORAGE, SKIN, FIELDS...>& b) noexcept(
            noexcept(!(a < b)))
{ return !(a < b); }

#endif // SOAVIEW_H

// vim: sw=4:tw=78:ft=cpp:et
