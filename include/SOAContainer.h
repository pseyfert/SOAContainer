/** @file SOAContainer.h
 *
 * @author Manuel Schiller <Manuel.Schiller@cern.ch>
 * @date 2015-04-10
 */

#ifndef SOACONTAINER_H
#define SOACONTAINER_H

#include <limits>
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
#include "SOAView.h"

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
class SOAContainer : public SOAView<
            typename SOATypelist::to_tuple<SOATypelist::typelist<FIELDS...>
                     >::template container_tuple<CONTAINER>,
            SKIN, FIELDS...>
{
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

        /// give a short and convenient name to base class
        typedef SOAView<
            typename SOATypelist::to_tuple<SOATypelist::typelist<FIELDS...>
                     >::template container_tuple<CONTAINER>,
            SKIN, FIELDS...> BASE;
    public:
        using size_type = typename BASE::size_type;
        using difference_type = typename BASE::difference_type;
        using pointer = typename BASE::pointer;
        using const_pointer = typename BASE::const_pointer;
        using iterator = typename BASE::iterator;
        using const_iterator = typename BASE::const_iterator;
        using reference = typename BASE::reference;
        using const_reference = typename BASE::const_reference;
        using reverse_iterator = typename BASE::reverse_iterator;
        using const_reverse_iterator = typename BASE::const_reverse_iterator;
        using value_type = typename BASE::value_type;
        using value_reference = typename BASE::value_reference;
        using value_const_reference = typename BASE::value_const_reference;
        /// type to represent container itself
        typedef SOAContainer<CONTAINER, SKIN, FIELDS...> self_type;
        /// typedef holding a typelist with the given fields
        typedef SOATypelist::typelist<FIELDS...> fields_typelist;

        /// convenience function to return member number given member tag type
        template <typename MEMBER>
        static constexpr size_type memberno() noexcept
        { return fields_typelist::template find<MEMBER>(); }

    protected:
        using naked_value_tuple_type = typename BASE::naked_value_tuple_type;
        using naked_reference_tuple_type = typename BASE::naked_reference_tuple_type;
        using naked_const_reference_tuple_type = typename BASE::naked_const_reference_tuple_type;
        using SOAStorage = typename BASE::SOAStorage;

    public:
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
        SOAContainer(const self_type& other) : BASE(other) { }
        /// move constructor
        SOAContainer(self_type&& other) : BASE(std::move(other)) { }

        /// std::initializer_list constructor
        SOAContainer(std::initializer_list<naked_value_tuple_type> listing) {
            reserve(listing.size());
            for(const auto &items : listing)
                push_back(items);
        }

        /// assignment from other SOAContainer
        self_type& operator=(const self_type& other)
        {
            if (&other != this) BASE::m_storage = other.m_storage;
            return *this;
        }
        /// move-assignment from other SOAContainer
        self_type& operator=(self_type&& other)
        {
            if (&other != this) BASE::m_storage = std::move(other.m_storage);
            return *this;
        }

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
                template <typename T>
                void operator()(T& obj) const noexcept(
                    noexcept(obj.reserve(m_sz)))
                { obj.reserve(m_sz); }
            };

            /// little helper for indexing to implement capacity()
            struct capacityHelper {
                template <typename T>
                size_type operator()(const T& obj) const noexcept(
                    noexcept(obj.capacity()))
                { return obj.capacity(); }
            };

            /// little helper for indexing to implement max_size()
            struct max_sizeHelper {
                template <typename T>
                size_type operator()(const T& obj) const noexcept(
                    noexcept(obj.max_size()))
                { return obj.max_size(); }
            };

            /// little helper for resize(sz) and resize(sz, val)
            struct resizeHelper {
                size_type m_sz;
                template <typename T>
                void operator()(T& obj) const noexcept(
                        noexcept(obj.resize(m_sz)))
                { obj.resize(m_sz); }

                template <typename T, typename V>
                void operator()(std::tuple<T&, const V&> t) const noexcept(
                        noexcept(std::get<0>(t).resize(m_sz, std::get<1>(t))))
                { std::get<0>(t).resize(m_sz, std::get<1>(t)); }
            };

            /// little helper for push_back
            struct push_backHelper {
                template <typename T, typename V>
                void operator()(std::tuple<T&, const V&> t) const noexcept(noexcept(
                            std::get<0>(t).push_back(std::get<1>(t))))
                { std::get<0>(t).push_back(std::get<1>(t)); }
                template <typename T, typename V>
                void operator()(std::tuple<T&, V&&> t) const noexcept(noexcept(
                        std::get<0>(t).push_back(std::move(std::get<1>(t)))))
                { std::get<0>(t).push_back(std::move(std::get<1>(t))); }
            };

            /// little helper for insert(it, val)
            struct insertHelper {
                size_type m_idx;
                template <typename T, typename V>
                void operator()(std::tuple<T&, const V&> t) const noexcept(
                        noexcept(std::get<0>(t).insert(std::get<0>(t).begin() + m_idx, std::get<1>(t))))
                { std::get<0>(t).insert(std::get<0>(t).begin() + m_idx, std::get<1>(t)); }
                template <typename T, typename V>
                void operator()(std::tuple<T&, V&&> t) const noexcept(
                        noexcept(std::get<0>(t).insert(std::get<0>(t).begin() + m_idx, std::move(std::get<1>(t)))))
                { std::get<0>(t).insert(std::get<0>(t).begin() + m_idx, std::move(std::get<1>(t))); }
            };

            /// little helper for insert(it, count, val)
            struct insertHelper2 {
                size_type m_idx;
                size_type m_count;
                template <typename T, typename V>
                void operator()(std::tuple<T&, const V&> t) const noexcept(
                        noexcept(std::get<0>(t).insert(std::get<0>(t).begin() + m_idx, m_count, std::get<1>(t))))
                { std::get<0>(t).insert(std::get<0>(t).begin() + m_idx, m_count, std::get<1>(t)); }
            };

            /// little helper for erase(it)
            struct eraseHelper {
                size_type m_idx;
                template <typename T>
                void operator()(T& obj) const noexcept(noexcept(
                        obj.erase(obj.begin() + m_idx)))
                { obj.erase(obj.begin() + m_idx); }
            };

            /// little helper for erase(first, last)
            struct eraseHelper_N {
                size_type m_idx;
                size_type m_len;
                template <typename T>
                void operator()(T& obj) const noexcept(noexcept(
                        obj.erase(obj.begin() + m_idx,
                            obj.begin() + m_idx + m_len)))
                { obj.erase(obj.begin() + m_idx, obj.begin() + m_idx + m_len); }
            };

            /// little helper for assign(count, val)
            struct assignHelper {
                size_type m_cnt;
                template <typename T, typename V>
                void operator()(std::tuple<T&, const V&> t) const noexcept(noexcept(
                        std::get<0>(t).assign(m_cnt, std::get<1>(t))))
                { std::get<0>(t).assign(m_cnt, std::get<1>(t)); }
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
            SOAUtils::map(typename impl_detail::clearHelper(), BASE::m_storage)))
        {
            SOAUtils::map(typename impl_detail::clearHelper(), BASE::m_storage);
        }

        /// pop the last element off the container
        void pop_back() noexcept(noexcept(
            SOAUtils::map(
                    typename impl_detail::pop_backHelper(), BASE::m_storage)))
        {
            SOAUtils::map(
                    typename impl_detail::pop_backHelper(), BASE::m_storage);
        }

        /// shrink the underlying storage of the container to fit its size
        void shrink_to_fit() noexcept(noexcept(
            SOAUtils::map(
                    typename impl_detail::shrink_to_fitHelper(), BASE::m_storage)))
        {
            SOAUtils::map(
                    typename impl_detail::shrink_to_fitHelper(), BASE::m_storage);
        }

        /// reserve space for at least sz elements
        void reserve(size_type sz) noexcept(noexcept(
            SOAUtils::map(
                    typename impl_detail::reserveHelper{sz}, BASE::m_storage)))
        {
            SOAUtils::map(
                    typename impl_detail::reserveHelper{sz}, BASE::m_storage);
        }

        /// return capacity of container
        size_type capacity() const
        {
            return SOAUtils::foldl<size_type>(
                    [] (size_type a, size_type b) noexcept
                    { return std::min(a, b); },
                    SOAUtils::map(
                        typename impl_detail::capacityHelper(), BASE::m_storage),
                    std::numeric_limits<size_type>::max());
        }

        /// return maximal size of container
        size_type max_size() const
        {
            return SOAUtils::foldl<size_type>(
                    [] (size_type a, size_type b) noexcept
                    { return std::min(a, b); },
                    SOAUtils::map(
                        typename impl_detail::max_sizeHelper(), BASE::m_storage),
                    std::numeric_limits<size_type>::max());
        }

        /// resize container (use default-constructed values if container grows)
        void resize(size_type sz) noexcept(noexcept(
                    SOAUtils::map(
                        typename impl_detail::resizeHelper{sz}, BASE::m_storage)))
        {
            SOAUtils::map(
                    typename impl_detail::resizeHelper{sz}, BASE::m_storage);
        }

        /// resize the container (append val if the container grows)
        void resize(size_type sz, const value_type& val) noexcept(noexcept(
            SOAUtils::map(typename impl_detail::resizeHelper{sz},
                    SOAUtils::zip(BASE::m_storage, val),
                    std::make_index_sequence<sizeof...(FIELDS)>())))
        {
            SOAUtils::map(typename impl_detail::resizeHelper{sz},
                    SOAUtils::zip(BASE::m_storage, val),
                    std::make_index_sequence<sizeof...(FIELDS)>());
        }

        /// push an element at the back of the array
        void push_back(const value_type& val) noexcept(noexcept(
            SOAUtils::map(typename impl_detail::push_backHelper(),
                    SOAUtils::zip(BASE::m_storage, val),
                    std::make_index_sequence<sizeof...(FIELDS)>())))
        {
            SOAUtils::map(typename impl_detail::push_backHelper(),
                    SOAUtils::zip(BASE::m_storage, val),
                    std::make_index_sequence<sizeof...(FIELDS)>());
        }

        /// push an element at the back of the array (move variant)
        void push_back(value_type&& val) noexcept(noexcept(
            SOAUtils::map(typename impl_detail::push_backHelper(),
                    SOAUtils::zip(BASE::m_storage, std::move(val)),
                    std::make_index_sequence<sizeof...(FIELDS)>())))
        {
            SOAUtils::map(typename impl_detail::push_backHelper(),
                    SOAUtils::zip(BASE::m_storage, std::move(val)),
                    std::make_index_sequence<sizeof...(FIELDS)>());
        }

        /// insert a value at the given position
        iterator insert(const_iterator pos, const value_type& val) noexcept(
                noexcept(SOAUtils::map(
                    typename impl_detail::insertHelper{pos.m_proxy.m_index},
                        SOAUtils::zip(BASE::m_storage, val),
                        std::make_index_sequence<sizeof...(FIELDS)>())))
        {
            assert((*pos).m_storage == &BASE::m_storage);
            SOAUtils::map(
                    typename impl_detail::insertHelper{pos.m_proxy.m_index},
                    SOAUtils::zip(BASE::m_storage, val),
                    std::make_index_sequence<sizeof...(FIELDS)>());
            return { pos.m_proxy.m_storage, pos.m_proxy.m_index };
        }

        /// insert a value at the given position (move variant)
        iterator insert(const_iterator pos, value_type&& val) noexcept(
                noexcept(SOAUtils::map(
                    typename impl_detail::insertHelper{pos.m_proxy.m_index},
                        SOAUtils::zip(BASE::m_storage, std::move(val)),
                        std::make_index_sequence<sizeof...(FIELDS)>())))
        {
            assert((*pos).m_storage == &BASE::m_storage);
            SOAUtils::map(
                    typename impl_detail::insertHelper{pos.m_proxy.m_index},
                    SOAUtils::zip(BASE::m_storage, std::move(val)),
                    std::make_index_sequence<sizeof...(FIELDS)>());
            return { pos.m_proxy.m_storage, pos.m_proxy.m_index };
        }

        /// insert count copies of value at the given position
        iterator insert(const_iterator pos, size_type count, const value_type& val) noexcept(
                noexcept(SOAUtils::map(
                    typename impl_detail::insertHelper2{pos.m_proxy.m_index, count},
                    SOAUtils::zip(BASE::m_storage, val),
                    std::make_index_sequence<sizeof...(FIELDS)>())))
        {
            assert((*pos).m_storage == &BASE::m_storage);
            SOAUtils::map(
                    typename impl_detail::insertHelper2{pos.m_proxy.m_index, count},
                    SOAUtils::zip(BASE::m_storage, val),
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
                    SOAUtils::map(
                        typename impl_detail::eraseHelper{pos.m_proxy.m_index},
                        BASE::m_storage)))
        {
            assert((*pos).m_storage == &BASE::m_storage);
            SOAUtils::map(
                    typename impl_detail::eraseHelper{pos.m_proxy.m_index},
                    BASE::m_storage);
            return { pos.m_proxy.m_storage, pos.m_proxy.m_index };
        }

        /// erase elements from first to last
        iterator erase(const_iterator first, const_iterator last) noexcept(
                noexcept(
                    SOAUtils::map(
                        typename impl_detail::eraseHelper_N{
                            first.m_proxy.m_index,
                            last.m_proxy.m_index - first.m_proxy.m_index},
                            BASE::m_storage)))
        {
            assert((*first).m_storage == &BASE::m_storage);
            assert((*last).m_storage == &BASE::m_storage);
            SOAUtils::map(
                    typename impl_detail::eraseHelper_N{first.m_proxy.m_index,
                        last.m_proxy.m_index - first.m_proxy.m_index},
                        BASE::m_storage);
            return { first.m_proxy.m_storage, first.m_proxy.m_index };
        }

        /// assign the vector to contain count copies of val
        void assign(size_type count, const value_type& val) noexcept(noexcept(
                    SOAUtils::map(typename impl_detail::assignHelper{count},
                        SOAUtils::zip(BASE::m_storage, val),
                        std::make_index_sequence<sizeof...(FIELDS)>())))
        {
            SOAUtils::map(typename impl_detail::assignHelper{count},
                    SOAUtils::zip(BASE::m_storage, val),
                    std::make_index_sequence<sizeof...(FIELDS)>());
        }

        /// assign the vector from a range of elements in another container
        template <typename IT>
        void assign(IT first, IT last) noexcept(
                noexcept(BASE::empty()) && noexcept(BASE::clear()) &&
                noexcept(insert(BASE::begin(), first, last)))
        {
            if (!BASE::empty()) BASE::clear();
            // naively, one would use a reserve(distance(first, last)) here,
            // but I'm not sure how this will work for various kinds of
            // special iterators - callers are expected to reserve beforehand
            // if the required size is known
            insert(BASE::begin(), first, last);
        }

        /// construct new element at end of container (in-place) from args
        template <typename... ARGS>
        void emplace_back(ARGS&&... args) noexcept(noexcept(
                    typename impl_detail::template emplace_backHelper<0>(
                        BASE::m_storage).doIt(std::forward<ARGS>(args)...)))
        {
            static_assert(std::is_constructible<value_type, ARGS...>::value,
                    "Wrong arguments to emplace_back.");
            typename impl_detail::template emplace_backHelper<0>(
                    BASE::m_storage).doIt(std::forward<ARGS>(args)...);
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
                        BASE::m_storage, pos.m_proxy.m_index).doIt(
                            std::forward<ARGS>(args)...)))
        {
            static_assert(std::is_constructible<value_type, ARGS...>::value,
                    "Wrong arguments to emplace.");
            assert(&BASE::m_storage == (*pos).m_storage);
            typename impl_detail::template emplaceHelper<0>(
                    BASE::m_storage, pos.m_proxy.m_index).doIt(
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
};

#endif // SOACONTAINER_H

// vim: sw=4:tw=78:ft=cpp:et
