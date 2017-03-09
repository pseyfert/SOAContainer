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

template <typename PROXY>
class SOAConstIterator;
template <typename PROXY>
class SOAIterator;
template < template <typename...> class CONTAINER,
         template <typename> class SKIN, typename... FIELDS>
class SOAContainer;

/** @brief proxy object for the elements stored in the container.
 *
 * @author Manuel Schiller <Manuel.Schiller@cern.ch>
 * @date 2015-04-10
 *
 * Conceptually, the SOAContainer contains a collection of objects
 * which have some data members. To optimise the data access patterns
 * in memory, the SOAContainer doesn't store the objects themselves,
 * but containers which each store a different member. That means the
 * conceptual objects mentioned above do not exist as such. The
 * SOAObjectProxy class stands in for these objects, and provides
 * support for accessing members, assigment of the whole conceptual
 * object from a tuple of its members, and similar functionality.
 */
template <typename PARENTCONTAINER>
class SOAObjectProxy {
    public:
        /// type of parent container
        typedef PARENTCONTAINER parent_type;
        /// type to refer to this type
        typedef SOAObjectProxy<PARENTCONTAINER> self_type;
        /// type to hold the distance between two iterators
        typedef typename parent_type::difference_type difference_type;
        /// type to hold the size of a container
        typedef typename parent_type::size_type size_type;
        /// type to which SOAObjectProxy converts and can be assigned from
        typedef typename parent_type::value_type value_type;
        /// typedef for tuple of references to members
        typedef typename parent_type::value_reference reference;
        /// typedef for tuple of const references to members
        typedef typename parent_type::value_const_reference const_reference;
        /// typedef to identify the type of a pointer
        typedef SOAIterator<typename parent_type::proxy> pointer;
        /// typedef to identify the type of a const pointer
        typedef SOAConstIterator<typename parent_type::proxy> const_pointer;

    protected:
        /// type used by the parent container to hold the SOA data
        typedef typename parent_type::SOAStorage SOAStorage;
        /// typelist of fields
        typedef typename parent_type::fields_typelist fields_typelist;

        SOAStorage* m_storage;  ///< underlying SOA storage of members
        size_type m_index;      ///< index into underlying SOA storage

        // SOAContainer is allowed to invoke the private constructor
        friend parent_type;
        /// corresponding SOAContainers are friends
        template < template <typename...> class CONTAINER,
                 template <typename> class SKIN, typename... FIELDS>
        friend class SOAContainer;
        // so is the pointer/iterator type
        friend pointer;
        // and the const pointer/iterator type
        friend const_pointer;

        /// constructor is private, but parent container is a friend
        SOAObjectProxy(
                SOAStorage* storage = nullptr, size_type index = 0) noexcept :
            m_storage(storage), m_index(index)
        { }

    private:
        /// little helper to implement conversion to tuple
        struct helper {
            size_type m_idx;
            /// convert to tuple of values
            template <typename T, std::size_t... IDX>
            auto to_value(const T& obj, std::index_sequence<IDX...>) const
                noexcept(noexcept(std::make_tuple(std::get<IDX>(obj)[m_idx]...)))
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
            auto to_const_reference(const T& obj, std::index_sequence<IDX...>) const
                noexcept(noexcept(std::tie(std::get<IDX>(obj)[m_idx]...)))
                -> decltype(std::tie(std::get<IDX>(obj)[m_idx]...))
            { return std::tie(std::get<IDX>(obj)[m_idx]...); }
        };

        /// little helper to implement concatenation of tuples
        struct tuplecatHelper {
            template <typename... S, typename T>
            auto operator()(
                    std::tuple<S...>&& t1, std::tuple<T>&& t2) const noexcept(
                    noexcept(std::tuple_cat(std::move(t1), std::move(t2)))) ->
                decltype(std::tuple_cat(std::move(t1), std::move(t2)))
            { return std::tuple_cat(std::move(t1), std::move(t2)); }
        };

        struct swapHelper {
            size_type m_idx1;
            size_type m_idx2;
            template <typename T>
            void operator()(std::tuple<T&, T&> obj) const noexcept(noexcept(
                        std::swap(std::get<0>(obj)[m_idx1], std::get<1>(obj)[m_idx2])))
            { std::swap(std::get<0>(obj)[m_idx1], std::get<1>(obj)[m_idx2]); }
        };

    public:
        /// copy constructor
        SOAObjectProxy(const self_type& other) noexcept = default;
        /// move constructor
        SOAObjectProxy(self_type&& other) noexcept = default;

        /// convert to tuple of member contents
        operator value_type() const noexcept(noexcept(
                    helper{ m_index }.to_value(*m_storage,
                        std::make_index_sequence<fields_typelist::size()>())))
        {
            return helper{ m_index }.to_value(*m_storage,
                    std::make_index_sequence<fields_typelist::size()>());
        }
        /// convert to tuple of references to members
        operator reference() noexcept(noexcept(
                    helper{ m_index }.to_reference(*m_storage,
                        std::make_index_sequence<fields_typelist::size()>())))
        {
            return helper{ m_index }.to_reference(*m_storage,
                    std::make_index_sequence<fields_typelist::size()>());
        }
        /// convert to tuple of const references to members
        operator const_reference() const noexcept(noexcept(
                    helper{ m_index }.to_const_reference(*m_storage,
                        std::make_index_sequence<fields_typelist::size()>())))
        {
            return helper{ m_index }.to_const_reference(*m_storage,
                    std::make_index_sequence<fields_typelist::size()>());
        }

        /// assign from tuple of member contents
        self_type& operator=(const value_type& other) noexcept(noexcept(
                    reference(self_type()) == other))
        { reference(*this) = other; return *this; }

        /// assign from tuple of member contents (move semantics)
        self_type& operator=(value_type&& other) noexcept(noexcept(
                    reference(self_type()) == std::move(other)))
        { reference(*this) = std::move(other); return *this; }

        /// assign from tuple of member contents
        self_type& operator=(const reference& other) noexcept(noexcept(
                    reference(self_type()) == other))
        { reference(*this) = other; return *this; }

        /// assign from tuple of member contents (move semantics)
        self_type& operator=(reference&& other) noexcept(noexcept(
                    reference(self_type()) == std::move(other)))
        { reference(*this) = std::move(other); return *this; }

        /// assign from tuple of member contents
        self_type& operator=(const const_reference& other) noexcept(noexcept(
                    reference(self_type()) == other))
        { reference(*this) = other; return *this; }

        /// assignment operator (value semantics)
        self_type& operator=(const self_type& other) noexcept(noexcept(
                    reference(self_type()) = const_reference(other)))
        {
            if (other.m_storage != m_storage || other.m_index != m_index)
                reference(*this) = const_reference(other);
            return *this;
        }

        /// move assignment operator (value semantics)
        self_type& operator=(self_type&& other) noexcept(noexcept(
                    reference(self_type()) = std::move(reference(other))))
        {
            if (other.m_storage != m_storage || other.m_index != m_index)
                reference(*this) = std::move(reference(other));
            return *this;
        }

        /// assignment (pointer-like semantics)
        self_type& assign(const self_type& other) noexcept
        {
            if (this != std::addressof(other))
                m_storage = other.m_storage, m_index = other.m_index;
            return *this;
        }
        /// move assignment (pointer-like semantics)
        self_type& assign(self_type&& other) noexcept
        {
            if (this != std::addressof(other))
                m_storage = std::move(other.m_storage),
                          m_index = std::move(other.m_index);
            return *this;
        }

        /// access to member by number
        template <size_type MEMBERNO>
        auto get() noexcept -> decltype(std::get<MEMBERNO>(*m_storage)[m_index])
        { return std::get<MEMBERNO>(*m_storage)[m_index]; }
        /// access to member by "member tag"
        template <typename MEMBER, size_type MEMBERNO =
            PARENTCONTAINER::template memberno<MEMBER>()>
        auto get() noexcept -> decltype(std::get<MEMBERNO>(*m_storage)[m_index])
        {
            static_assert(PARENTCONTAINER::template memberno<MEMBER>() ==
                    MEMBERNO, "Called with wrong template argument(s).");
            return std::get<MEMBERNO>(*m_storage)[m_index];
        }
        /// access to member by number (read-only)
        template <size_type MEMBERNO>
        auto get() const noexcept -> decltype(std::get<MEMBERNO>(*m_storage)[m_index])
        { return std::get<MEMBERNO>(*m_storage)[m_index]; }
        /// access to member by "member tag" (read-only)
        template <typename MEMBER, size_type MEMBERNO =
            PARENTCONTAINER::template memberno<MEMBER>()>
        auto get() const noexcept -> decltype(std::get<MEMBERNO>(*m_storage)[m_index])
        {
            static_assert(PARENTCONTAINER::template memberno<MEMBER>() ==
                    MEMBERNO, "Called with wrong template argument(s).");
            return std::get<MEMBERNO>(*m_storage)[m_index];
        }

        /// swap the contents of two SOAObjectProxy instances
        void swap(self_type& other) noexcept(noexcept(
                    SOAUtils::map(swapHelper{ m_index, other.m_index },
                    SOAUtils::zip(*m_storage, *other.m_storage),
                    std::make_index_sequence<fields_typelist::size()>())))
        {
            SOAUtils::map(swapHelper{ m_index, other.m_index },
                    SOAUtils::zip(*m_storage, *other.m_storage),
                    std::make_index_sequence<fields_typelist::size()>());
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
        pointer operator&() noexcept;
        /// return const pointer to element pointed to be this proxy
        const_pointer operator&() const noexcept;
};

/// comparison (equality)
template <typename T>
bool operator==(const typename SOAObjectProxy<T>::value_type& a,
        const SOAObjectProxy<T>& b) noexcept { return b == a; }
/// comparison (inequality)
template <typename T>
bool operator!=(const typename SOAObjectProxy<T>::value_type& a,
        const SOAObjectProxy<T>& b) noexcept { return b != a; }
/// comparison (less than)
template <typename T>
bool operator<(const typename SOAObjectProxy<T>::value_type& a,
        const SOAObjectProxy<T>& b) noexcept { return b > a; }
/// comparison (greater than)
template <typename T>
bool operator>(const typename SOAObjectProxy<T>::value_type& a,
        const SOAObjectProxy<T>& b) noexcept { return b < a; }
/// comparison (less than or equal to)
template <typename T>
bool operator<=(const typename SOAObjectProxy<T>::value_type& a,
        const SOAObjectProxy<T>& b) noexcept { return b >= a; }
/// comparison (greater than or equal to)
template <typename T>
bool operator>=(const typename SOAObjectProxy<T>::value_type& a,
        const SOAObjectProxy<T>& b) noexcept { return b <= a; }

namespace std {
    /// specialise std::swap for SOAObjectProxy<T>
    template <typename T>
    void swap(SOAObjectProxy<T> a, SOAObjectProxy<T> b) noexcept(
            noexcept(a.swap(b)))
    { a.swap(b); }
}

#include "SOAIterator.h"

template <typename PARENTCONTAINER>
typename SOAObjectProxy<PARENTCONTAINER>::pointer
SOAObjectProxy<PARENTCONTAINER>::operator&() noexcept
{ return { m_storage, m_index }; }

template <typename PARENTCONTAINER>
typename SOAObjectProxy<PARENTCONTAINER>::const_pointer
SOAObjectProxy<PARENTCONTAINER>::operator&() const noexcept
{ return { m_storage, m_index }; }


#endif // SOAOBJECTPROXY_H

// vim: sw=4:tw=78:ft=cpp:et
