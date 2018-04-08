/** @file SOAObjectProxy.h
 *
 * @author Manuel Schiller <Manuel.Schiller@cern.ch>
 * @date 2015-04-10
 */

#ifndef SOAOBJECTPROXY_H
#define SOAOBJECTPROXY_H

#include <tuple>
#include <memory>

#include "SOATypelist.h"
#include "SOATypelistUtils.h"
#include "SOAUtils.h"
#include "c++14_compat.h"

/// namespace to encapsulate SOA stuff
namespace SOA {
    // forward declarations
    template <class STORAGE,
             template <typename> class SKIN, typename... FIELDS>
                 class _View;
    template <template <typename...> class CONTAINER,
             template <typename> class SKIN, typename... FIELDS>
                 class _Container;
} // namespace SOA

namespace SOA {
    /** @brief proxy object for the elements stored in the container.
     *
     * @author Manuel Schiller <Manuel.Schiller@cern.ch>
     * @date 2015-04-10
     *
     * Conceptually, the _Container contains a collection of objects
     * which have some data members. To optimise the data access patterns
     * in memory, the _Container doesn't store the objects themselves,
     * but containers which each store a different member. That means the
     * conceptual objects mentioned above do not exist as such. The
     * ObjectProxy class stands in for these objects, and provides
     * support for accessing members, assigment of the whole conceptual
     * object from a tuple of its members, and similar functionality.
     */
    template <typename POSITION>
    class ObjectProxy : protected POSITION {
        public:
            /// type of parent container
            using parent_type = typename POSITION::parent_type;
            /// type to refer to this type
            using self_type = ObjectProxy<POSITION>;
            /// type to hold the distance between two iterators
            using difference_type = typename parent_type::difference_type;
            /// type to hold the size of a container
            using size_type = typename parent_type::size_type;
            /// type to which ObjectProxy converts and can be assigned from
            using value_type = typename parent_type::value_type;
            /// type for tuple of references to members
            using reference = typename parent_type::value_reference;
            /// type for tuple of const references to members
            using const_reference = typename parent_type::value_const_reference;
            /// type of a pointer
            using pointer = typename parent_type::pointer;
            /// type of a const pointer
            using const_pointer = typename parent_type::const_pointer;

        protected:
            /// type used by the parent container to hold the SOA data
            using SOAStorage = typename parent_type::SOAStorage;
            /// typelist of fields
            using fields_typelist = typename parent_type::fields_typelist;

            using POSITION::stor;
            using POSITION::idx;

            /// corresponding _Containers are friends
            template <template <typename...> class CONTAINER,
                     template <typename> class SKIN, typename... FIELDS>
            friend class _Container;

        public:
            // magic constructor
            template <typename POS, typename =
                typename std::enable_if<
                std::is_base_of<POSITION, POS>::value>::type>
            constexpr explicit ObjectProxy(POS&& pos) noexcept :
                POSITION(std::forward<POS>(pos))
            {}

        private:
            /// little helper to implement conversion to tuple
            struct helper {
                size_type m_idx;
                /// convert to tuple of values
                template <typename T, std::size_t... IDX>
                auto to_value(const T& obj, std::index_sequence<IDX...>) const
                    noexcept(noexcept(std::make_tuple(
                                    std::get<IDX>(obj)[m_idx]...)))
                    -> decltype(std::make_tuple(std::get<IDX>(obj)[m_idx]...))
                { return std::make_tuple(std::get<IDX>(obj)[m_idx]...); }
                /// convert to tuple of references
                template <typename T, std::size_t... IDX>
                auto to_reference(T& obj, std::index_sequence<IDX...>) const
                    noexcept(noexcept(std::tie(std::get<IDX>(obj)[m_idx]...)))
                    -> decltype(std::tie(std::get<IDX>(obj)[m_idx]...))
                { return std::tie(std::get<IDX>(obj)[m_idx]...); }
                /// convert to tuple of const references
                template <typename T, std::size_t... IDX>
                auto to_const_reference(
                        const T& obj, std::index_sequence<IDX...>) const
                    noexcept(noexcept(std::tie(std::get<IDX>(obj)[m_idx]...)))
                    -> decltype(std::tie(std::get<IDX>(obj)[m_idx]...))
                { return std::tie(std::get<IDX>(obj)[m_idx]...); }
            };

            struct swapHelper {
                size_type m_idx1;
                size_type m_idx2;
                template <typename T>
                void operator()(std::tuple<T&, T&> obj) const noexcept(
                        noexcept(std::swap(std::get<0>(obj)[m_idx1],
                                std::get<1>(obj)[m_idx2])))
                {
                    std::swap(std::get<0>(obj)[m_idx1],
                            std::get<1>(obj)[m_idx2]);
                }
            };

        public:
            /// default constructor
            ObjectProxy() = default;
            /// copy constructor
            ObjectProxy(const self_type& other) = default;
            /// move constructor
            ObjectProxy(self_type&& other) = default;

            /// convert to tuple of member contents
            operator value_type() const noexcept(noexcept(
                        helper{ std::declval<self_type>().idx() }.to_value(
                                *std::declval<self_type>().stor(),
                            std::make_index_sequence<
                            fields_typelist::size()>())))
            {
                return helper{ idx() }.to_value(*stor(),
                        std::make_index_sequence<fields_typelist::size()>());
            }
            /// convert to tuple of references to members
            operator reference() noexcept(noexcept(
                        helper{ std::declval<self_type>().idx() }.to_reference(
                                *std::declval<self_type>().stor(),
                            std::make_index_sequence<
                            fields_typelist::size()>())))
            {
                return helper{ idx() }.to_reference(*stor(),
                        std::make_index_sequence<fields_typelist::size()>());
            }
            /// convert to tuple of const references to members
            operator const_reference() const noexcept(noexcept(
                        helper{ std::declval<self_type>().idx() }.to_const_reference(
                                *std::declval<self_type>().stor(),
                            std::make_index_sequence<
                            fields_typelist::size()>())))
            {
                return helper{ idx() }.to_const_reference(*stor(),
                        std::make_index_sequence<
                        fields_typelist::size()>());
            }

            /// assign from tuple of member contents
            template <typename VALUE_TYPE>
            typename std::enable_if<std::is_same<value_type,
                     typename std::remove_cv<typename
                         std::remove_reference<VALUE_TYPE>::type>::type>::value,
            self_type>::type& operator=(VALUE_TYPE&& other) noexcept(noexcept(
                        reference(std::declval<self_type>()) ==
                        std::forward<VALUE_TYPE>(other)))
            { reference(*this) = std::forward<VALUE_TYPE>(other); return *this; }

            template <typename REFERENCE_TYPE>
            typename std::enable_if<std::is_same<reference, typename
                std::remove_cv<typename
                std::remove_reference<REFERENCE_TYPE>::type>::type>::value,
            self_type>::type& operator=(REFERENCE_TYPE&& other)
                noexcept(noexcept( reference(std::declval<self_type>()) ==
                            std::forward<REFERENCE_TYPE>(other)))
            {
                reference(*this) = std::forward<REFERENCE_TYPE>(other);
                return *this;
            }

            template <typename CONST_REFERENCE_TYPE>
            typename std::enable_if<std::is_same<const_reference, typename
                std::remove_cv<typename std::remove_reference<
                CONST_REFERENCE_TYPE>::type>::type>::value,
            self_type>::type& operator=(CONST_REFERENCE_TYPE&& other)
                noexcept(noexcept( reference(std::declval<self_type>()) ==
                            std::forward<CONST_REFERENCE_TYPE>(other)))
            {
                reference(*this) = std::forward<CONST_REFERENCE_TYPE>(other);
                return *this;
            }

            template <typename SELF_TYPE>
            typename std::enable_if<std::is_same<self_type, typename
                std::remove_cv<typename
                std::remove_reference<SELF_TYPE>::type>::type>::value,
            self_type>::type& operator=(SELF_TYPE&& other) noexcept(noexcept(
                        reference(std::declval<self_type>()) ==
                        std::forward<SELF_TYPE>(other)))
            {
                if (other.stor() != stor() || other.idx() != idx())
                    reference(*this) = std::forward<SELF_TYPE>(other);
                return *this;
            }

            /// assignment (pointer-like semantics)
            template <typename SELF_TYPE>
            typename std::enable_if<std::is_same<self_type, typename std::remove_cv<
                typename std::remove_reference<SELF_TYPE>::type>::type>::value,
            self_type>::type& assign(SELF_TYPE&& other) noexcept
            {
                if (this != std::addressof(other))
                    stor() = other.stor(), idx() = other.idx();
                return *this;
            }

            /// access to member by number
            template <size_type MEMBERNO>
            auto get() noexcept -> decltype(
                    std::get<MEMBERNO>(*std::declval<self_type>().stor())[
                    std::declval<self_type>().idx()])
            { return std::get<MEMBERNO>(*stor())[idx()]; }
            /// access to member by "member tag"
            template <typename MEMBER, size_type MEMBERNO =
                parent_type::template memberno<MEMBER>()>
            auto get() noexcept -> decltype(
                    std::get<MEMBERNO>(*std::declval<self_type>().stor())[
                    std::declval<self_type>().idx()])
            {
                static_assert(parent_type::template memberno<MEMBER>() ==
                        MEMBERNO, "Called with wrong template argument(s).");
                return std::get<MEMBERNO>(*stor())[idx()];
            }
            /// access to member by number (read-only)
            template <size_type MEMBERNO>
            auto get() const noexcept -> decltype(
                    std::get<MEMBERNO>(*std::declval<self_type>().stor())[
                    std::declval<self_type>().idx()])
            { return std::get<MEMBERNO>(*stor())[idx()]; }
            /// access to member by "member tag" (read-only)
            template <typename MEMBER, size_type MEMBERNO =
                parent_type::template memberno<MEMBER>()>
            auto get() const noexcept -> decltype(
                    std::get<MEMBERNO>(*std::declval<self_type>().stor())[
                    std::declval<self_type>().idx()])
            {
                static_assert(parent_type::template memberno<MEMBER>() ==
                        MEMBERNO, "Called with wrong template argument(s).");
                return std::get<MEMBERNO>(*stor())[idx()];
            }

            /// swap the contents of two ObjectProxy instances
            void swap(self_type& other) noexcept(noexcept(
                        SOA::Utils::map(swapHelper{ other.idx(), other.idx() },
                        SOA::Utils::zip(*other.stor(), *other.stor()))))
            {
                SOA::Utils::map(swapHelper{ idx(), other.idx() },
                        SOA::Utils::zip(*stor(), *other.stor()));
            }

            /// comparison (equality)
            bool operator==(const value_type& other) const noexcept
            { return const_reference(*this) == other; }
            /// comparison (inequality)
            bool operator!=(const value_type& other) const noexcept
            { return const_reference(*this) != other; }
            /// comparison (less than)
            bool operator<(const value_type& other) const noexcept
            { return const_reference(*this) < other; }
            /// comparison (greater than)
            bool operator>(const value_type& other) const noexcept
            { return const_reference(*this) > other; }
            /// comparison (less than or equal to)
            bool operator<=(const value_type& other) const noexcept
            { return const_reference(*this) <= other; }
            /// comparison (greater than or equal to)
            bool operator>=(const value_type& other) const noexcept
            { return const_reference(*this) >= other; }

            /// comparison (equality)
            bool operator==(const self_type& other) const noexcept
            { return const_reference(*this) == const_reference(other); }
            /// comparison (inequality)
            bool operator!=(const self_type& other) const noexcept
            { return const_reference(*this) != const_reference(other); }
            /// comparison (less than)
            bool operator<(const self_type& other) const noexcept
            { return const_reference(*this) < const_reference(other); }
            /// comparison (greater than)
            bool operator>(const self_type& other) const noexcept
            { return const_reference(*this) > const_reference(other); }
            /// comparison (less than or equal to)
            bool operator<=(const self_type& other) const noexcept
            { return const_reference(*this) <= const_reference(other); }
            /// comparison (greater than or equal to)
            bool operator>=(const self_type& other) const noexcept
            { return const_reference(*this) >= const_reference(other); }

            /// return pointer to element pointed to be this proxy
            pointer operator&() noexcept
            { return pointer{ POSITION(*this) }; }
            /// return const pointer to element pointed to be this proxy
            const_pointer operator&() const noexcept
            { return const_pointer{ POSITION(*this) }; }
    };

    /// comparison (equality)
    template <typename T>
    bool operator==(const typename ObjectProxy<T>::value_type& a,
            const ObjectProxy<T>& b) noexcept { return b == a; }
    /// comparison (inequality)
    template <typename T>
    bool operator!=(const typename ObjectProxy<T>::value_type& a,
            const ObjectProxy<T>& b) noexcept { return b != a; }
    /// comparison (less than)
    template <typename T>
    bool operator<(const typename ObjectProxy<T>::value_type& a,
            const ObjectProxy<T>& b) noexcept { return b > a; }
    /// comparison (greater than)
    template <typename T>
    bool operator>(const typename ObjectProxy<T>::value_type& a,
            const ObjectProxy<T>& b) noexcept { return b < a; }
    /// comparison (less than or equal to)
    template <typename T>
    bool operator<=(const typename ObjectProxy<T>::value_type& a,
            const ObjectProxy<T>& b) noexcept { return b >= a; }
    /// comparison (greater than or equal to)
    template <typename T>
    bool operator>=(const typename ObjectProxy<T>::value_type& a,
            const ObjectProxy<T>& b) noexcept { return b <= a; }

    /// helper for std::swap for ObjectProxy<T>
    template <typename T>
    void swap(ObjectProxy<T> a, ObjectProxy<T> b) noexcept(
            noexcept(a.swap(b)))
    { a.swap(b); }
} // namespace SOA

#endif // SOAOBJECTPROXY_H

// vim: sw=4:tw=78:ft=cpp:et
