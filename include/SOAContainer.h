/** @file SOAContainer.h
 *
 * @author Manuel Schiller <Manuel.Schiller@cern.ch>
 * @date 2015-04-10
 */

#ifndef SOACONTAINER_H
#define SOACONTAINER_H

#include <cassert>
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

/** @brief skin class for SOAContainer which does nothing and preserved the
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
                    static_cast<NAKEDPROXY*>(nullptr)->operator=(arg)))
        { NAKEDPROXY::operator=(arg); return *this; }

        /// move assignment operator - forward to underlying proxy
        template <typename ARG>
        NullSkin<NAKEDPROXY>& operator=(ARG&& arg) noexcept(noexcept(
                    static_cast<NAKEDPROXY*>(nullptr)->operator=(
                        std::move(arg))))
        { NAKEDPROXY::operator=(std::move(arg)); return *this; }
};

/** @brief container class for objects with given fields (SOA storage)
 *
 * @author Manuel Schiller <Manuel.Schiller@cern.ch>
 * @date 2015-04-10
 *
 * @tparam CONTAINER    underlying container type (anything that follows
 *                      std::vector's interface is fine)
 * @tparam SKIN         "skin" to dress the the interface of the object
 *                      proxying the content elemnts; this can be used to
 *                      augment the interface provided by the get<fieldtag>()
 *                      syntax with something more convenient; NullSkin leaves
 *                      the raw interface intact
 * @tparam FIELDS...    list of "field tags" describing names an types
 *                      of members
 *
 * This class represents a container of objects with the given list of fields,
 * the objects are not stored as such, but each of object's fields gets its own
 * storage array, effectively creating a structure-of-arrays (SOA) layout which
 * is advantageous for vectorisation of algorithms. To illustrate the SOA
 * layout, first consider the normal array-of-structures (AOS) layout:
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
 * #include "SOAContainer.h"
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
 * typedef SOAContainer<
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
 * SOAContainer will not create such an object at any point in time. Instead,
 * the container provides an interface to access fields with the get<field_tag>
 * syntax, or via tuples (conversion to or assignment from tuples is
 * implemented), if simultaneous access to all fields is desired.
 *
 * Apart from these points (which are dictated by the SOA layout and efficiency
 * considerations), the class tries to follow the example of the interface of
 * std::vector as closely as possible.
 *
 * When using the SOAContainer class, one must distinguish between elements
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
template < template <typename...> class CONTAINER,
    template <typename> class SKIN, typename... FIELDS>
class SOAContainer {
    private:
        /// hide verification of FIELDS inside struct or doxygen gets confused
        struct fields_verifier {
            // storing objects without state doesn't make sense
            static_assert(1 <= sizeof...(FIELDS),
                "need to supply at least one field");
            /// little helper to verify the FIELDS template parameter
            template <typename... ARGS>
            struct verify_fields;
            /// specialisation for more than one field
            template <typename HEAD, typename... TAIL>
            struct verify_fields<HEAD, TAIL...>
            {
                /// true if HEAD and TAIL verify okay
                enum {
                    value = (std::is_pod<HEAD>::value ||
                        SOATypelist::is_wrapped<
                        HEAD>::value) && verify_fields<TAIL...>::value
                };
            };
            /// specialisation for one field
            template <typename HEAD>
            struct verify_fields<HEAD>
            {
                /// true if HEAD verifies okay
                enum {
                    value = std::is_pod<HEAD>::value ||
                        SOATypelist::is_wrapped<HEAD>::value
                };
            };
            // make sure fields are either POD or wrapped types
            static_assert(verify_fields<FIELDS...>::value,
                "Fields should be either plain old data (POD) or "
                "wrapped types.");
        };

    public:
        /// type to represent sizes and indices
        typedef std::size_t size_type;
        /// type to represent differences of indices
        typedef std::ptrdiff_t difference_type;
        /// type to represent container itself
        typedef SOAContainer<CONTAINER, SKIN, FIELDS...> self_type;
        /// typedef holding a typelist with the given fields
        typedef SOATypelist::typelist<FIELDS...> fields_typelist;

        /// convenience function to return member number given member tag type
        template <typename MEMBER>
        static constexpr size_type memberno() noexcept
        { return fields_typelist::template find<MEMBER>(); }

    private:
        /// type of the storage backend
        typedef typename SOATypelist::to_tuple<
            fields_typelist>::template container_tuple<CONTAINER> SOAStorage;

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

        /// default constructor
        SOAContainer() { }
        /// fill container with count copies of val
        SOAContainer(size_type count, const value_type& val)
        {
            reserve(count);
            assign(count, val);
        }
        /// fill container with count (default constructed) elements
        SOAContainer(size_type count) : SOAContainer(count, value_type()) { }
        /// fill container with elements from other container
        template <typename IT>
        SOAContainer(IT first, IT last)
        { assign(first, last); }
        /// copy constructor
        SOAContainer(const self_type& other) : m_storage(other.m_storage) { }
        /// move constructor
        SOAContainer(self_type&& other) :
            m_storage(std::move(other.m_storage)) { }

        /// std::initializer_list constructor
        SOAContainer(std::initializer_list<naked_value_tuple_type> listing) {
            reserve(listing.size());
            for(const auto &items : listing)
                push_back(items);
        }

        /// assignment from other SOAContainer
        self_type& operator=(const self_type& other)
        {
            if (&other != this) m_storage = other.m_storage;
            return *this;
        }
        /// move-assignment from other SOAContainer
        self_type& operator=(self_type&& other)
        {
            if (&other != this) m_storage = std::move(other.m_storage);
            return *this;
        }

        /// return if the container is empty
        bool empty() const noexcept(noexcept(std::get<0>(m_storage).empty()))
        { return std::get<0>(m_storage).empty(); }
        /// return the size of the container
        size_type size() const noexcept(noexcept(
                    std::get<0>(m_storage).size()))
        { return std::get<0>(m_storage).size(); }

    private:
        /// hide implementation details in struct to make doxygen tidier
        struct impl_detail {
            /// little helper for indexing to implement clear()
            struct clearHelper {
                template <typename T>
                void operator()(T& obj) const noexcept(noexcept(obj.clear()))
                { obj.clear(); }
            };

            /// little helper for indexing to implement pop_back()
            struct pop_backHelper {
                template <typename T>
                void operator()(T& obj) const noexcept(
                        noexcept(obj.pop_back()))
                { obj.pop_back(); }
            };

            /// little helper for indexing to implement shrink_to_fit()
            struct shrink_to_fitHelper {
                template <typename T>
                void operator()(T& obj) const noexcept(
                        noexcept(obj.shrink_to_fit()))
                { obj.shrink_to_fit(); }
            };

            /// little helper for indexing to implement reserve()
            struct reserveHelper {
                size_type m_sz;
                reserveHelper(size_type sz) noexcept : m_sz(sz) { }
                template <typename T>
                void operator()(T& obj) const noexcept(
                    noexcept(obj.reserve(m_sz)))
                { obj.reserve(m_sz); }
            };

            /// little helper for indexing to implement capacity()
            struct capacityHelper {
                template <typename T, typename IDX>
                size_type operator()(T& obj, IDX) const noexcept(
                    noexcept(obj.capacity()))
                { return obj.capacity(); }
            };

            /// little helper for indexing to implement max_size()
            struct max_sizeHelper {
                template <typename T, typename IDX>
                size_type operator()(T& obj, IDX) const noexcept(
                    noexcept(obj.max_size()))
                { return obj.max_size(); }
            };

            /// little helper for resize(sz) and resize(sz, val)
            struct resizeHelper {
                size_type m_sz;
                resizeHelper(size_type sz) noexcept : m_sz(sz) { }
                template <typename T>
                void operator()(T& obj) const noexcept(
                        noexcept(obj.resize(m_sz)))
                { obj.resize(m_sz); }

                template <typename T, typename V>
                void operator()(T& obj, const V& val) const noexcept(
                        noexcept(obj.resize(m_sz, val)))
                { obj.resize(m_sz, val); }
            };

            /// little helper for push_back
            struct push_backHelper {
                template <typename T, typename V>
                void operator()(T& obj, const V& val) const noexcept(noexcept(
                            obj.push_back(val)))
                { obj.push_back(val); }
                template <typename T, typename V>
                void operator()(T& obj, V&& val) const noexcept(noexcept(
                        obj.push_back(std::move(val))))
                { obj.push_back(std::move(val)); }
            };

            /// little helper for insert(it, val)
            struct insertHelper {
                size_type m_idx;
                insertHelper(size_type idx) noexcept :
                    m_idx(idx) { }
                template <typename T, typename V>
                void operator()(T& obj, const V& val) const noexcept(
                        noexcept(obj.insert(obj.begin() + m_idx, val)))
                { obj.insert(obj.begin() + m_idx, val); }
                template <typename T, typename V>
                void operator()(T& obj, V&& val) const noexcept(
                        noexcept(obj.insert(obj.begin() + m_idx, std::move(val))))
                { obj.insert(obj.begin() + m_idx, std::move(val)); }
                template <typename T, typename V>
                void operator()(T& obj, const V& val, size_type count) const noexcept(
                        noexcept(obj.insert(obj.begin() + m_idx, val, count)))
                { obj.insert(obj.begin() + m_idx, count, val); }
            };

            /// little helper for erase(it)
            struct eraseHelper {
                size_type m_idx;
                eraseHelper(size_type idx) noexcept : m_idx(idx) { }
                template <typename T, typename IDX>
                void operator()(T& obj, IDX) const noexcept(noexcept(
                        obj.erase(obj.begin() + m_idx)))
                { obj.erase(obj.begin() + m_idx); }
            };

            /// little helper for erase(first, last)
            struct eraseHelper_N {
                size_type m_idx;
                size_type m_sz;
                eraseHelper_N(size_type idx, size_type sz) noexcept :
                m_idx(idx), m_sz(sz) { }
                template <typename T, typename IDX>
                void operator()(T& obj, IDX) const noexcept(noexcept(
                        obj.erase(obj.begin() + m_idx,
                            obj.begin() + m_idx + m_sz)))
                { obj.erase(obj.begin() + m_idx, obj.begin() + m_idx + m_sz); }
            };

            /// little helper for assign(count, val)
            struct assignHelper {
                const value_type& m_val;
                size_type m_cnt;
                assignHelper(
                    const value_type& val, size_type cnt) noexcept :
                m_val(val), m_cnt(cnt) { }
                template <typename T, typename IDX>
                void operator()(T& obj, IDX) const noexcept(noexcept(
                        obj.assign(m_cnt, std::get<IDX::value>(m_val))))
                { obj.assign(m_cnt, std::get<IDX::value>(m_val)); }
            };

            /// helper for emplace_back
            template <size_type IDX>
            struct emplace_backHelper {
                SOAStorage& m_storage;
                emplace_backHelper(SOAStorage& storage) noexcept :
                m_storage(storage) { }
                template <typename HEAD, typename... TAIL>
                void doIt(HEAD&& head, TAIL&&... tail) const noexcept(noexcept(
                        std::get<IDX>(m_storage).emplace_back(
                            std::forward<HEAD>(head))) && noexcept(
                        emplace_backHelper<IDX + 1>(m_storage).doIt(
                            std::forward<TAIL>(tail)...)))
                {
                    std::get<IDX>(m_storage).emplace_back(
                            std::forward<HEAD>(head));
                    emplace_backHelper<IDX + 1>(m_storage).doIt(
                            std::forward<TAIL>(tail)...);
                }
                template <typename HEAD>
                void doIt(HEAD&& head) const noexcept(noexcept(
                        std::get<IDX>(m_storage).emplace_back(
                            std::forward<HEAD>(head))))
                {
                    std::get<IDX>(m_storage).emplace_back(
                            std::forward<HEAD>(head));
                }
            };

            /// helper for emplace
            template <size_type IDX>
            struct emplaceHelper {
                SOAStorage& m_storage;
                size_type m_idx;
                emplaceHelper(SOAStorage& storage, size_type idx) noexcept :
                m_storage(storage), m_idx(idx) { }
                template <typename HEAD, typename... TAIL>
                void doIt(HEAD&& head, TAIL&&... tail) const noexcept(noexcept(
                        std::get<IDX>(m_storage).emplace(
                            std::get<IDX>(m_storage).begin() + m_idx,
                            std::forward<HEAD>(head))) && noexcept(
                        emplaceHelper<IDX + 1>(m_storage, m_idx).doIt(
                            std::forward<TAIL>(tail)...)))
                {
                    std::get<IDX>(m_storage).emplace(
                            std::get<IDX>(m_storage).begin() + m_idx,
                            std::forward<HEAD>(head));
                    emplaceHelper<IDX + 1>(m_storage, m_idx).doIt(
                            std::forward<TAIL>(tail)...);
                }
                template <typename HEAD>
                void doIt(HEAD&& head) const noexcept(noexcept(
                        std::get<IDX>(m_storage).emplace(
                            std::get<IDX>(m_storage).begin() + m_idx,
                            std::forward<HEAD>(head))))
                {
                    std::get<IDX>(m_storage).emplace(
                            std::get<IDX>(m_storage).begin() + m_idx,
                            std::forward<HEAD>(head));
                }
            };

            /// helper for emplace_back
            struct emplaceBackHelper2 {
                self_type* m_obj;
                emplaceBackHelper2(self_type* obj) : m_obj(obj) {}

                template <typename... ARGS>
                void operator()(ARGS&&... args) const noexcept(noexcept(
                    m_obj->emplace_back(std::forward<ARGS>(args)...)))
                { m_obj->emplace_back(std::forward<ARGS>(args)...); }
            };

            // helper for emplace
            struct emplaceHelper2 {
                self_type* m_obj;
                const_iterator m_it;
                emplaceHelper2(self_type* obj, const_iterator it) :
                    m_obj(obj), m_it(it) {}

                template <typename... ARGS>
                iterator operator()(ARGS&&... args) const noexcept(noexcept(
                    m_obj->emplace(m_it, std::forward<ARGS>(args)...)))
                { return m_obj->emplace(m_it, std::forward<ARGS>(args)...); }
            };
        }; // end of struct impl_detail

    public:
        /// clear the container
        void clear() noexcept(noexcept(
            SOAUtils::apply_tuple(m_storage, typename impl_detail::clearHelper(),
                std::make_index_sequence<sizeof...(FIELDS)>())))
        {
            SOAUtils::apply_tuple(m_storage, typename impl_detail::clearHelper(),
                    std::make_index_sequence<sizeof...(FIELDS)>());
        }

        /// pop the last element off the container
        void pop_back() noexcept(noexcept(
            SOAUtils::apply_tuple(m_storage,
                    typename impl_detail::pop_backHelper(),
                    std::make_index_sequence<sizeof...(FIELDS)>())))
        {
            SOAUtils::apply_tuple(m_storage,
                    typename impl_detail::pop_backHelper(),
                    std::make_index_sequence<sizeof...(FIELDS)>());
        }

        /// shrink the underlying storage of the container to fit its size
        void shrink_to_fit() noexcept(noexcept(
            SOAUtils::apply_tuple(m_storage,
                    typename impl_detail::shrink_to_fitHelper(),
                    std::make_index_sequence<sizeof...(FIELDS)>())))
        {
            SOAUtils::apply_tuple(m_storage,
                    typename impl_detail::shrink_to_fitHelper(),
                    std::make_index_sequence<sizeof...(FIELDS)>());
        }

        /// reserve space for at least sz elements
        void reserve(size_type sz) noexcept(noexcept(
            SOAUtils::apply_tuple(m_storage,
                    typename impl_detail::reserveHelper(sz),
                    std::make_index_sequence<sizeof...(FIELDS)>())))
        {
            SOAUtils::apply_tuple(m_storage,
                    typename impl_detail::reserveHelper(sz),
                    std::make_index_sequence<sizeof...(FIELDS)>());
        }

        /// return capacity of container
        size_type capacity() const
        {
            return SOAUtils::recursive_apply_tuple<sizeof...(FIELDS)>()(
                    m_storage, typename impl_detail::capacityHelper(),
                    [] (size_type s1, size_type s2) {
                    return std::min(s1, s2); }, size_type(-1));
        }

        /// return maximal size of container
        size_type max_size() const
        {
            return SOAUtils::recursive_apply_tuple<sizeof...(FIELDS)>()(
                    m_storage, typename impl_detail::max_sizeHelper(),
                    [] (size_type s1, size_type s2) {
                    return std::min(s1, s2); }, size_type(-1));
        }

        /// access specified element
        reference operator[](size_type idx) noexcept
        { return { &m_storage, idx }; }
        /// access specified element (read access only)
        const_reference operator[](size_type idx) const noexcept
        { return { &const_cast<SOAStorage&>(m_storage), idx }; }
        /// access specified element with out of bounds checking
        reference at(size_type idx)
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
        reference front() noexcept { return operator[](0); }
        /// access first element (non-empty container, read-only)
        const_reference front() const noexcept { return operator[](0); }
        /// access last element (non-empty container)
        reference back() noexcept(noexcept(
                    static_cast<self_type*>(nullptr)->size()))
        { return operator[](size() - 1); }
        /// access last element (non-empty container, read-only)
        const_reference back() const noexcept(noexcept(
                    static_cast<self_type*>(nullptr)->size()))
        { return operator[](size() - 1); }

        /// iterator pointing to first element
        iterator begin() noexcept { return { &m_storage, 0 }; }
        /// const iterator pointing to first element
        const_iterator begin() const noexcept
        { return { const_cast<SOAStorage*>(&m_storage), 0 }; }
        /// const iterator pointing to first element
        const_iterator cbegin() const noexcept { return begin(); }

        /// iterator pointing one element behind the last element
        iterator end() noexcept { return { &m_storage, size() }; }
        /// const iterator pointing one element behind the last element
        const_iterator end() const noexcept
        { return { const_cast<SOAStorage*>(&m_storage), size() }; }
        /// const iterator pointing one element behind the last element
        const_iterator cend() const noexcept { return end(); }

        /// get begin iterator of storage vector for member MEMBERNO
        template <size_type MEMBERNO>
        auto begin() noexcept -> decltype(
                std::get<MEMBERNO>(m_storage).begin())
        { return std::get<MEMBERNO>(m_storage).begin(); }

        /// get begin iterator of storage vector for member with tag MEMBER
        template <typename MEMBER>
        auto begin() noexcept -> decltype(
                std::get<memberno<MEMBER>()>(m_storage).begin())
        { return std::get<memberno<MEMBER>()>(m_storage).begin(); }

        /// get begin iterator of storage vector for member MEMBERNO
        template <size_type MEMBERNO>
        auto begin() const noexcept -> decltype(
                std::get<MEMBERNO>(m_storage).begin())
        { return std::get<MEMBERNO>(m_storage).begin(); }

        /// get begin iterator of storage vector for member with tag MEMBER
        template <typename MEMBER>
        auto begin() const noexcept -> decltype(
                std::get<memberno<MEMBER>()>(m_storage).begin())
        { return std::get<memberno<MEMBER>()>(m_storage).begin(); }

        /// get begin iterator of storage vector for member MEMBERNO
        template <size_type MEMBERNO>
        auto cbegin() const noexcept -> decltype(
                std::get<MEMBERNO>(m_storage).cbegin())
        { return std::get<MEMBERNO>(m_storage).cbegin(); }

        /// get begin iterator of storage vector for member with tag MEMBER
        template <typename MEMBER>
        auto cbegin() const noexcept -> decltype(
                std::get<memberno<MEMBER>()>(m_storage).cbegin())
        { return std::get<memberno<MEMBER>()>(m_storage).cbegin(); }

        /// get end iterator of storage vector for member MEMBERNO
        template <size_type MEMBERNO>
        auto end() noexcept -> decltype(
                std::get<MEMBERNO>(m_storage).end())
        { return std::get<MEMBERNO>(m_storage).end(); }

        /// get end iterator of storage vector for member with tag MEMBER
        template <typename MEMBER>
        auto end() noexcept -> decltype(
                std::get<memberno<MEMBER>()>(m_storage).end())
        { return std::get<memberno<MEMBER>()>(m_storage).end(); }

        /// get end iterator of storage vector for member MEMBERNO
        template <size_type MEMBERNO>
        auto end() const noexcept -> decltype(
                std::get<MEMBERNO>(m_storage).end())
        { return std::get<MEMBERNO>(m_storage).end(); }

        /// get end iterator of storage vector for member with tag MEMBER
        template <typename MEMBER>
        auto end() const noexcept -> decltype(
                std::get<memberno<MEMBER>()>(m_storage).end())
        { return std::get<memberno<MEMBER>()>(m_storage).end(); }

        /// get cend iterator of storage vector for member MEMBERNO
        template <size_type MEMBERNO>
        auto cend() const noexcept -> decltype(
                std::get<MEMBERNO>(m_storage).cend())
        { return std::get<MEMBERNO>(m_storage).cend(); }

        /// get cend iterator of storage vector for member with tag MEMBER
        template <typename MEMBER>
        auto cend() const noexcept -> decltype(
                std::get<memberno<MEMBER>()>(m_storage).cend())
        { return std::get<memberno<MEMBER>()>(m_storage).cend(); }

        /// iterator pointing to first element in reverse order
        reverse_iterator rbegin() noexcept { return reverse_iterator(end()); }
        /// const iterator pointing to first element in reverse order
        const_reverse_iterator rbegin() const noexcept
        { return const_reverse_iterator(end()); }
        /// const iterator pointing to first element in reverse order
        const_reverse_iterator crbegin() const noexcept { return rbegin(); }

        /// iterator pointing one element behind the last element in reverse order
        reverse_iterator rend() noexcept { return reverse_iterator(begin()); }
        /// const iterator pointing one element behind the last element in reverse order
        const_reverse_iterator rend() const noexcept
        { return const_reverse_iterator(begin()); }
        /// const iterator pointing one element behind the last element in reverse order
        const_reverse_iterator crend() const noexcept { return rend(); }

        /// get rbegin iterator of storage vector for member MEMBERNO
        template <size_type MEMBERNO>
        auto rbegin() noexcept -> decltype(
                std::get<MEMBERNO>(m_storage).rbegin())
        { return std::get<MEMBERNO>(m_storage).rbegin(); }

        /// get rbegin iterator of storage vector for member with tag MEMBER
        template <typename MEMBER>
        auto rbegin() noexcept -> decltype(
                std::get<memberno<MEMBER>()>(m_storage).rbegin())
        { return std::get<memberno<MEMBER>()>(m_storage).rbegin(); }

        /// get rbegin iterator of storage vector for member MEMBERNO
        template <size_type MEMBERNO>
        auto rbegin() const noexcept -> decltype(
                std::get<MEMBERNO>(m_storage).rbegin())
        { return std::get<MEMBERNO>(m_storage).rbegin(); }

        /// get rbegin iterator of storage vector for member with tag MEMBER
        template <typename MEMBER>
        auto rbegin() const noexcept -> decltype(
                std::get<memberno<MEMBER>()>(m_storage).rbegin())
        { return std::get<memberno<MEMBER>()>(m_storage).rbegin(); }

        /// get rbegin iterator of storage vector for member MEMBERNO
        template <size_type MEMBERNO>
        auto crbegin() const noexcept -> decltype(
                std::get<MEMBERNO>(m_storage).crbegin())
        { return std::get<MEMBERNO>(m_storage).crbegin(); }

        /// get rbegin iterator of storage vector for member with tag MEMBER
        template <typename MEMBER>
        auto crbegin() const noexcept -> decltype(
                std::get<memberno<MEMBER>()>(m_storage).crbegin())
        { return std::get<memberno<MEMBER>()>(m_storage).crbegin(); }

        /// get rend iterator of storage vector for member MEMBERNO
        template <size_type MEMBERNO>
        auto rend() noexcept -> decltype(
                std::get<MEMBERNO>(m_storage).rend())
        { return std::get<MEMBERNO>(m_storage).rend(); }

        /// get rend iterator of storage vector for member with tag MEMBER
        template <typename MEMBER>
        auto rend() noexcept -> decltype(
                std::get<memberno<MEMBER>()>(m_storage).rend())
        { return std::get<memberno<MEMBER>()>(m_storage).rend(); }

        /// get rend iterator of storage vector for member MEMBERNO
        template <size_type MEMBERNO>
        auto rend() const noexcept -> decltype(
                std::get<MEMBERNO>(m_storage).rend())
        { return std::get<MEMBERNO>(m_storage).rend(); }

        /// get rend iterator of storage vector for member with tag MEMBER
        template <typename MEMBER>
        auto rend() const noexcept -> decltype(
                std::get<memberno<MEMBER>()>(m_storage).rend())
        { return std::get<memberno<MEMBER>()>(m_storage).rend(); }

        /// get crend iterator of storage vector for member MEMBERNO
        template <size_type MEMBERNO>
        auto crend() const noexcept -> decltype(
                std::get<MEMBERNO>(m_storage).crend())
        { return std::get<MEMBERNO>(m_storage).crend(); }

        /// get crend iterator of storage vector for member with tag MEMBER
        template <typename MEMBER>
        auto crend() const noexcept -> decltype(
                std::get<memberno<MEMBER>()>(m_storage).crend())
        { return std::get<memberno<MEMBER>()>(m_storage).crend(); }

        /// resize container (use default-constructed values if container grows)
        void resize(size_type sz) noexcept(noexcept(
                    SOAUtils::apply_tuple(
                        m_storage, typename impl_detail::resizeHelper(sz),
                        std::make_index_sequence<sizeof...(FIELDS)>())))
        {
            SOAUtils::apply_tuple(m_storage,
                    typename impl_detail::resizeHelper(sz),
                    std::make_index_sequence<sizeof...(FIELDS)>());
        }

        /// resize the container (append val if the container grows)
        void resize(size_type sz, const value_type& val) noexcept(noexcept(
                    SOAUtils::apply_tuple2(m_storage,
                        typename impl_detail::resizeHelper(sz), val,
                        std::make_index_sequence<sizeof...(FIELDS)>())))
        {
            SOAUtils::apply_tuple2(m_storage,
                    typename impl_detail::resizeHelper(sz), val,
                    std::make_index_sequence<sizeof...(FIELDS)>());
        }

        /// push an element at the back of the array
        void push_back(const value_type& val) noexcept(noexcept(
                    SOAUtils::apply_tuple2(m_storage,
                        typename impl_detail::push_backHelper(), val,
                        std::make_index_sequence<sizeof...(FIELDS)>())))
        {
            SOAUtils::apply_tuple2(m_storage,
                    typename impl_detail::push_backHelper(), val,
                    std::make_index_sequence<sizeof...(FIELDS)>());
        }

        /// push an element at the back of the array (move variant)
        void push_back(value_type&& val) noexcept(noexcept(
                    SOAUtils::apply_tuple2(m_storage,
                        typename impl_detail::push_backHelper(),
                        std::move(val),
                        std::make_index_sequence<sizeof...(FIELDS)>())))
        {
            SOAUtils::apply_tuple2(m_storage,
                    typename impl_detail::push_backHelper(), std::move(val),
                    std::make_index_sequence<sizeof...(FIELDS)>());
        }

        /// insert a value at the given position
        iterator insert(const_iterator pos, const value_type& val) noexcept(
                noexcept(SOAUtils::apply_tuple2(m_storage,
                    typename impl_detail::insertHelper(pos.m_proxy.m_index),
                    val, std::make_index_sequence<sizeof...(FIELDS)>())))
        {
            assert((*pos).m_storage == &m_storage);
            SOAUtils::apply_tuple2(m_storage,
                    typename impl_detail::insertHelper(pos.m_proxy.m_index),
                    val, std::make_index_sequence<sizeof...(FIELDS)>());
            return { pos.m_proxy.m_storage, pos.m_proxy.m_index };
        }

        /// insert a value at the given position (move variant)
        iterator insert(const_iterator pos, value_type&& val) noexcept(
                noexcept(SOAUtils::apply_tuple2(m_storage,
                    typename impl_detail::insertHelper(pos.m_proxy.m_index),
                    std::move(val),
                    std::make_index_sequence<sizeof...(FIELDS)>())))
        {
            assert((*pos).m_storage == &m_storage);
            SOAUtils::apply_tuple2(m_storage,
                    typename impl_detail::insertHelper(pos.m_proxy.m_index),
                    std::move(val),
                    std::make_index_sequence<sizeof...(FIELDS)>());
            return { pos.m_proxy.m_storage, pos.m_proxy.m_index };
        }

        /// insert count copies of value at the given position
        iterator insert(const_iterator pos, size_type count, const value_type& val) noexcept(
                noexcept(SOAUtils::apply_tuple3(m_storage,
                    typename impl_detail::insertHelper(pos.m_proxy.m_index),
                    val, count,
                    std::make_index_sequence<sizeof...(FIELDS)>())))
        {
            assert((*pos).m_storage == &m_storage);
            SOAUtils::apply_tuple3(m_storage,
                    typename impl_detail::insertHelper(pos.m_proxy.m_index),
                    val, count,
                    std::make_index_sequence<sizeof...(FIELDS)>());
            return { pos.m_proxy.m_storage, pos.m_proxy.m_index };
        }

        /// insert elements between first and last at position pos
        template <typename IT>
        iterator insert(const_iterator pos, IT first, IT last)
        {
            // FIXME: terrible implementation!!!
            // for iterators which support multiple passes over the data, this
            // should fill field by field
            // moreover, if we can determine the distance between first and
            // last, we should make a hole of the right size to avoid moving
            // data more than once
            iterator retVal(pos.m_proxy.m_storage, pos.m_proxy.m_index);
            while (first != last) { insert(pos, *first); ++first; ++pos; }
            return retVal;
        }

        /// erase an element at the given position
        iterator erase(const_iterator pos) noexcept(noexcept(
                    SOAUtils::recursive_apply_tuple<sizeof...(FIELDS)>()(
                        m_storage, typename impl_detail::eraseHelper(
                            pos.m_proxy.m_index))))
        {
            assert((*pos).m_storage == &m_storage);
            SOAUtils::recursive_apply_tuple<sizeof...(FIELDS)>()(m_storage,
                    typename impl_detail::eraseHelper(pos.m_proxy.m_index));
            return { pos.m_proxy.m_storage, pos.m_proxy.m_index };
        }

        /// erase elements from first to last
        iterator erase(const_iterator first, const_iterator last) noexcept(
                noexcept(
                    SOAUtils::recursive_apply_tuple<sizeof...(FIELDS)>()(
                        m_storage, typename impl_detail::eraseHelper_N(
                            first.m_proxy.m_index,
                            last.m_proxy.m_index - first.m_proxy.m_index))))
        {
            assert((*first).m_storage == &m_storage);
            assert((*last).m_storage == &m_storage);
            SOAUtils::recursive_apply_tuple<sizeof...(FIELDS)>()(m_storage,
                    typename impl_detail::eraseHelper_N(first.m_proxy.m_index,
                        last.m_proxy.m_index - first.m_proxy.m_index));
            return { first.m_proxy.m_storage, first.m_proxy.m_index };
        }

        /// assign the vector to contain count copies of val
        void assign(size_type count, const value_type& val) noexcept(noexcept(
                    SOAUtils::recursive_apply_tuple<sizeof...(FIELDS)>()(
                        m_storage,
                        typename impl_detail::assignHelper(val, count))))
        {
            SOAUtils::recursive_apply_tuple<sizeof...(FIELDS)>()(m_storage,
                    typename impl_detail::assignHelper(val, count));
        }

        /// assign the vector from a range of elements in another container
        template <typename IT>
        void assign(IT first, IT last) noexcept(
                noexcept(empty()) && noexcept(clear()) &&
                noexcept(insert(begin(), first, last)))
        {
            if (!empty()) clear();
            // naively, one would use a reserve(distance(first, last)) here,
            // but I'm not sure how this will work for various kinds of
            // special iterators - callers are expected to reserve beforehand
            // if the required size is known
            insert(begin(), first, last);
        }

        /// construct new element at end of container (in-place) from args
        template <typename... ARGS>
        void emplace_back(ARGS&&... args) noexcept(noexcept(
                    typename impl_detail::template emplace_backHelper<0>(
                        m_storage).doIt(std::forward<ARGS>(args)...)))
        {
            static_assert(std::is_constructible<value_type, ARGS...>::value,
                    "Wrong arguments to emplace_back.");
            typename impl_detail::template emplace_backHelper<0>(
                    m_storage).doIt(std::forward<ARGS>(args)...);
        }

        /// construct a new element at the end of container from naked_value_tuple_type
        void emplace_back(naked_value_tuple_type&& val) noexcept(noexcept(
            SOAUtils::call(typename impl_detail::emplaceBackHelper2(nullptr),
                std::forward<naked_value_tuple_type>(val))))
        {
            return SOAUtils::call(
                typename impl_detail::emplaceBackHelper2(this),
                std::forward<naked_value_tuple_type>(val));
        }

        /// construct a new element at the end of container from value_type
        void emplace_back(value_type&& val) noexcept(noexcept(
            SOAUtils::call(typename impl_detail::emplaceBackHelper2(nullptr),
                std::forward<naked_value_tuple_type>(
                    static_cast<naked_value_tuple_type>(val)))))
        {
            return SOAUtils::call(
                typename impl_detail::emplaceBackHelper2(this),
                std::forward<naked_value_tuple_type>(
                    static_cast<naked_value_tuple_type>(val)));
        }

        /// construct new element at position pos (in-place) from args
        template <typename... ARGS>
        iterator emplace(const_iterator pos, ARGS&&... args) noexcept(
                noexcept(typename impl_detail::template emplaceHelper<0>(
                        m_storage, pos.m_proxy.m_index).doIt(
                            std::forward<ARGS>(args)...)))
        {
            static_assert(std::is_constructible<value_type, ARGS...>::value,
                    "Wrong arguments to emplace.");
            assert(&m_storage == (*pos).m_storage);
            typename impl_detail::template emplaceHelper<0>(
                    m_storage, pos.m_proxy.m_index).doIt(
                        std::forward<ARGS>(args)...);
            return { pos.m_proxy.m_storage, pos.m_proxy.m_index };
        }

        /// construct a new element at position pos from naked_value_tuple_type
        iterator emplace(const_iterator pos,
            naked_value_tuple_type&& val) noexcept(noexcept(
                SOAUtils::call(typename impl_detail::emplaceHelper2(
                nullptr, pos), std::forward<naked_value_tuple_type>(val))))
        {
            return SOAUtils::call(
                typename impl_detail::emplaceHelper2(this, pos),
                std::forward<naked_value_tuple_type>(val));
        }

        /// construct a new element at position pos from value_type
        iterator emplace(const_iterator pos,
            value_type&& val) noexcept(noexcept(
                SOAUtils::call(typename impl_detail::emplaceHelper2(
                nullptr, pos), std::forward<naked_value_tuple_type>(
                    static_cast<naked_value_tuple_type>(val)))))
        {
            return SOAUtils::call(
                typename impl_detail::emplaceHelper2(this, pos),
                std::forward<naked_value_tuple_type>(
                    static_cast<naked_value_tuple_type>(val)));
        }

        /// construct new element at position pos (in-place) from args
        template <typename... ARGS>
        /// swap contents of two containers
        void swap(self_type& other) noexcept(
                noexcept(std::swap(m_storage, other.m_storage)))
        { std::swap(m_storage, other.m_storage); }
};

namespace std {
    /// specialise std::swap
    template <template <typename...> class CONTAINER,
             template <typename> class SKIN,
             typename... FIELDS>
    void swap(const SOAContainer<CONTAINER, SKIN, FIELDS...>& a,
            const SOAContainer<CONTAINER, SKIN, FIELDS...>& b) noexcept(
                noexcept(a.swap(b)))
    { a.swap(b); }
}

/// more SOAContainer implementation details
namespace SOAContainerImpl {
    /// helper class to compare SOAContainers (field by field)
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

    /// helper class to compare SOAContainers, specialised N = 1
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

/// compare two SOAContainers for equality
template < template <typename...> class CONTAINER,
    template <typename> class SKIN, typename... FIELDS>
bool operator==(const SOAContainer<CONTAINER, SKIN, FIELDS...>& a,
        const SOAContainer<CONTAINER, SKIN, FIELDS...>& b) noexcept(
            noexcept(a.size()) && noexcept(
                SOAContainerImpl::compare<std::equal_to,
                SOAContainer<CONTAINER, SKIN, FIELDS...
                >::fields_typelist::size()>()(a, b)))
{
    if (a.size() != b.size()) return false;
    // compare one field at a time
    return SOAContainerImpl::compare<std::equal_to,
           SOAContainer<CONTAINER, SKIN, FIELDS...
               >::fields_typelist::size()>()(a, b);
}

/// compare two SOAContainers for inequality
template < template <typename...> class CONTAINER,
    template <typename> class SKIN, typename... FIELDS>
bool operator!=(const SOAContainer<CONTAINER, SKIN, FIELDS...>& a,
        const SOAContainer<CONTAINER, SKIN, FIELDS...>& b) noexcept(
            noexcept(a.size()) && noexcept(
                SOAContainerImpl::compare<std::not_equal_to,
                SOAContainer<CONTAINER, SKIN, FIELDS...
                >::fields_typelist::size()>()(a, b)))
{
    if (a.size() != b.size()) return true;
    // compare one field at a time
    return SOAContainerImpl::compare<std::not_equal_to,
           SOAContainer<CONTAINER, SKIN, FIELDS...
               >::fields_typelist::size()>()(a, b);
}

/// compare two SOAContainers lexicographically using <
template < template <typename...> class CONTAINER,
    template <typename> class SKIN, typename... FIELDS>
bool operator<(const SOAContainer<CONTAINER, SKIN, FIELDS...>& a,
        const SOAContainer<CONTAINER, SKIN, FIELDS...>& b) noexcept(
            noexcept(std::lexicographical_compare(a.cbegin(), a.cend(),
                    b.cbegin(), b.cend(),
                    std::less<decltype(a.front())>())))
{
    return std::lexicographical_compare(a.cbegin(), a.cend(),
            b.cbegin(), b.cend(), std::less<decltype(a.front())>());
}

/// compare two SOAContainers lexicographically using >
template < template <typename...> class CONTAINER,
    template <typename> class SKIN, typename... FIELDS>
bool operator>(const SOAContainer<CONTAINER, SKIN, FIELDS...>& a,
        const SOAContainer<CONTAINER, SKIN, FIELDS...>& b) noexcept(
            noexcept(b < a))
{ return b < a; }

/// compare two SOAContainers lexicographically using <=
template < template <typename...> class CONTAINER,
    template <typename> class SKIN, typename... FIELDS>
bool operator<=(const SOAContainer<CONTAINER, SKIN, FIELDS...>& a,
        const SOAContainer<CONTAINER, SKIN, FIELDS...>& b) noexcept(
            noexcept(!(a > b)))
{ return !(a > b); }

/// compare two SOAContainers lexicographically using >=
template < template <typename...> class CONTAINER,
    template <typename> class SKIN, typename... FIELDS>
bool operator>=(const SOAContainer<CONTAINER, SKIN, FIELDS...>& a,
        const SOAContainer<CONTAINER, SKIN, FIELDS...>& b) noexcept(
            noexcept(!(a < b)))
{ return !(a < b); }

#endif // SOACONTAINER_H

// vim: sw=4:tw=78:ft=cpp:et
