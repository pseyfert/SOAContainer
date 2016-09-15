/** @file SOAObjectProxy.h
 *
 * @author Manuel Schiller <Manuel.Schiller@cern.ch>
 * @date 2015-04-10
 */

#ifndef SOAOBJECTPROXY_H
#define SOAOBJECTPROXY_H

#include <tuple>
#include <memory>

#include "SOATuplePrinter.h"
#include "SOATypelist.h"
#include "SOATypelistUtils.h"
#include "SOAUtils.h"

template <typename PROXY>
class SOAConstIterator;
template <typename PROXY>
class SOAIterator;

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

	SOAStorage* m_storage;	///< underlying SOA storage of members
	size_type m_index;	///< index into underlying SOA storage

	// SOAContainer is allowed to invoke the private constructor
	friend parent_type;
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
	struct to_valueHelper {
	    size_type m_idx;
	    template <typename T, typename Idx>
	    std::tuple<typename T::value_type>
	    operator()(const T& obj, Idx) const noexcept(noexcept(
			std::tuple<typename T::value_type>(obj[m_idx])))
	    { return std::tuple<typename T::value_type>(obj[m_idx]); }
	};

	/// little helper to implement conversion to tuple of references
	struct to_referenceHelper {
	    size_type m_idx;
	    template <typename T, typename Idx>
	    std::tuple<typename T::reference>
	    operator()(T& obj, Idx) const noexcept(noexcept(
			    std::tuple<typename T::reference>(obj[m_idx])))
	    { return std::tuple<typename T::reference>(obj[m_idx]); }
	};

	/// little helper to implement conversion to tuple of const references
	struct to_const_referenceHelper {
	    size_type m_idx;
	    template <typename T, typename Idx>
	    std::tuple<typename T::const_reference>
	    operator()(const T& obj, Idx) const noexcept(noexcept(
			std::tuple<const typename T::const_reference>(obj[m_idx])))
	    { return std::tuple<const typename T::const_reference>(obj[m_idx]); }
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
	    SOAStorage* m_other;
	    template <typename T, typename Idx>
	    void operator()(T& obj, Idx) const noexcept(noexcept(
			std::swap(obj[m_idx1],
			    std::get<Idx::value>(*m_other)[m_idx2])))
	    {
		std::swap(obj[m_idx1], std::get<Idx::value>(*m_other)[m_idx2]);
	    }
	};

    public:
	/// copy constructor
	SOAObjectProxy(const self_type& other) noexcept :
	    m_storage(other.m_storage), m_index(other.m_index) { }
	/// move constructor
	SOAObjectProxy(self_type&& other) noexcept :
	    m_storage(std::move(other.m_storage)),
	    m_index(std::move(other.m_index)) { }

	/// convert to tuple of member contents
	operator value_type() const noexcept(noexcept(
		    SOAUtils::recursive_apply_tuple<
		    fields_typelist::size()>()(
			*m_storage, to_valueHelper({ m_index }),
			tuplecatHelper(), std::tuple<>())))
	{
	    return SOAUtils::recursive_apply_tuple<
		fields_typelist::size()>()(
		    *m_storage, to_valueHelper({ m_index }),
		    tuplecatHelper(), std::tuple<>());
	}

	/// convert to tuple of references member contents
	operator reference() noexcept(noexcept(
		    SOAUtils::recursive_apply_tuple<
		    fields_typelist::size()>()(
			*m_storage, to_referenceHelper({ m_index }),
			tuplecatHelper(), std::tuple<>())))
	{
	    return SOAUtils::recursive_apply_tuple<
		fields_typelist::size()>()(
		    *m_storage, to_referenceHelper({ m_index }),
		    tuplecatHelper(), std::tuple<>());
	}

	/// convert to tuple of const references member contents
	operator const_reference() const noexcept(noexcept(
		    SOAUtils::recursive_apply_tuple<
		    fields_typelist::size()>()(
			*m_storage, to_const_referenceHelper({ m_index }),
			tuplecatHelper(), std::tuple<>())))
	{
	    return SOAUtils::recursive_apply_tuple<
		fields_typelist::size()>()(
		    *m_storage, to_const_referenceHelper({ m_index }),
		    tuplecatHelper(), std::tuple<>());
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
	template <typename MEMBER>
	auto get() noexcept -> decltype(std::get<
		PARENTCONTAINER::template memberno<MEMBER>()>(
		    *m_storage)[m_index])
	{
	    return std::get<
		PARENTCONTAINER::template memberno<MEMBER>()>(
			*m_storage)[m_index];
	}
	/// access to member by number (read-only)
	template <size_type MEMBERNO>
	auto get() const noexcept -> decltype(std::get<MEMBERNO>(*m_storage)[m_index])
	{ return std::get<MEMBERNO>(*m_storage)[m_index]; }
	/// access to member by "member tag" (read-only)
	template <typename MEMBER>
	auto get() const noexcept -> decltype(std::get<
		PARENTCONTAINER::template memberno<MEMBER>()>(
		    *m_storage)[m_index])
	{
	    return std::get<
		PARENTCONTAINER::template memberno<MEMBER>()>(
			*m_storage)[m_index];
	}

	/// swap the contents of two SOAObjectProxy instances
	void swap(self_type& other) noexcept(noexcept(
		    SOAUtils::recursive_apply_tuple<fields_typelist::size()>()(
		    *m_storage,
		    swapHelper({ m_index, other.m_index, other.m_storage }))))
	{
	    SOAUtils::recursive_apply_tuple<fields_typelist::size()>()(
		    *m_storage,
		    swapHelper({ m_index, other.m_index, other.m_storage }));
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

    
//using TupleHelper::operator<<;

template <typename T>
std::ostream& operator<< (std::ostream& os, const SOAObjectProxy<T> &prox) {
    const typename SOAObjectProxy<T>::value_type as_tuple = prox;
    TupleHelper::print_tuple(os, as_tuple);
    return os; // << prox;
};

#endif // SOAOBJECTPROXY_H

// vim: sw=4:tw=78:ft=cpp
