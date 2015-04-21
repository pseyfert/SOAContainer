/** @file SOAObjectProxy.h
 *
 * @author Manuel Schiller <Manuel.Schiller@cern.ch>
 * @date 2015-04-10
 */

#ifndef SOAOBJECTPROXY_H
#define SOAOBJECTPROXY_H

#include <tuple>
#include "SOATypelist.h"
#include "SOATypelistUtils.h"
#include "SOAUtils.h"

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
 *
 * At the same time, the SOAObjectProxy acts like a pointer to one of
 * the contained objects (this was done mainly to facilitate writing
 * iterator classes).
 *
 * Hence, a SOAObjectProxy is a strange entity which leads its
 * existence as a cross between a (smart) reference and a (smart)
 * pointer...
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
	typedef typename parent_type::value_tuple_type value_type;
	/// typedef for tuple of references to members
	typedef typename parent_type::reference_tuple_type reference_type;

    private:
	/// type used by the parent container to hold the SOA data
	typedef typename parent_type::SOAStorage SOAStorage;
	/// typelist of fields
	typedef typename parent_type::fields_typelist fields_typelist;

	size_type m_index;	///< index into underlying SOA storage
	SOAStorage* m_storage;	///< underlying SOA storage of members

    protected:
	// SOAContainer is allowed to invoke the private constructor
	friend parent_type;
	/// constructor is private, but parent container is a friend
	SOAObjectProxy(
		SOAStorage* storage = nullptr, size_type index = 0) noexcept :
	    m_index(index), m_storage(storage)
        { }

    private:
	/// little helper to implement conversion to tuple
	struct to_valueHelper {
	    size_type m_idx;
	    template <typename T, typename Idx>
		std::tuple<typename T::value_type>
		operator()(const T& obj, Idx) const
		{ return std::tuple<typename T::value_type>(obj[m_idx]); }
	};

	/// little helper to implement conversion to tuple of references
	struct to_referenceHelper {
	    size_type m_idx;
	    template <typename T, typename Idx>
		std::tuple<typename T::value_type&>
		operator()(const T& obj, Idx) const
		{ return std::tuple<typename T::value_type&>(obj[m_idx]); }
	};

	/// little helper to implement concatenation of tuples
	struct tuplecatHelper {
	    template <typename... S, typename T>
		auto operator()(std::tuple<S...>&& t1, std::tuple<T>&& t2) const ->
		decltype(std::tuple_cat(std::move(t1), std::move(t2)))
		{ return std::tuple_cat(std::move(t1), std::move(t2)); }
	};

    public:
	/// copy constructor
	SOAObjectProxy(const self_type& other) :
	    m_index(other.m_index), m_storage(other.m_storage) { }
	/// move constructor
	SOAObjectProxy(self_type&& other) :
	    m_index(std::move(other.m_index)),
	    m_storage(std::move(other.m_storage)) { }
	/// assignment operator
	self_type& operator=(const self_type& other)
	{
	    if (&other != this)
		m_index = other.m_index, m_storage = other.m_storage;
	    return *this;
	}
	/// move assignment operator
	self_type& operator=(self_type&& other)
	{
	    if (&other != this)
		m_index = std::move(other.m_index),
			m_storage = std::move(other.m_storage);
	    return *this;
	}

	/// access to member by number
	template <size_type MEMBERNO>
	auto get() -> decltype(std::get<MEMBERNO>(*m_storage)[m_index])
	{ return std::get<MEMBERNO>(*m_storage)[m_index]; }
	/// access to member by "member tag"
	template <typename MEMBER>
        auto get() -> decltype(std::get<SOATypelist::find<
		fields_typelist,
		MEMBER>::index>(*m_storage)[m_index])
	{
	    return std::get<SOATypelist::find<fields_typelist,
	        MEMBER>::index>(*m_storage)[m_index];
	}
	/// access to member by number (read-only)
	template <size_type MEMBERNO>
	auto get() const -> decltype(std::get<MEMBERNO>(*m_storage)[m_index])
	{ return std::get<MEMBERNO>(*m_storage)[m_index]; }
	/// access to member by "member tag" (read-only)
	template <typename MEMBER>
	auto get() const -> decltype(std::get<SOATypelist::find<
		    fields_typelist,
		    MEMBER>::index>(*m_storage)[m_index])
	{
	    return std::get<SOATypelist::find<fields_typelist,
	        MEMBER>::index>(*m_storage)[m_index];
	}

	/// convert to tuple of member contents
	operator value_type() const
	{
	    return SOAUtils::recursive_apply_tuple<
		fields_typelist::size()>()(
		    *m_storage, to_valueHelper({ m_index }),
		    tuplecatHelper(), std::tuple<>());
	}

	/// convert to tuple of references member contents
	operator reference_type() const
	{
	    return SOAUtils::recursive_apply_tuple<
		fields_typelist::size()>()(
		    *m_storage, to_referenceHelper({ m_index }),
		    tuplecatHelper(), std::tuple<>());
	}

	/// assign from tuple of member contents
	self_type& operator=(const value_type& other)
	{
	    reference_type(*this) = other;
	    return *this;
	}

	/// assign from tuple of member contents
	self_type& operator=(const reference_type& other)
	{
	    reference_type(*this) = other;
	    return *this;
	}

	/// "deep"-swap the contents of two SOAObjectProxy instances
	void swap(self_type& other)
	{ std::swap(reference_type(*this), reference_type(other)); }

	/// dereference
	self_type& operator*() noexcept
	{ return *this; }
	/// dereference
	const self_type& operator*() const noexcept
	{ return *this; }
	/// helper to support iterator->get<...>() idiom
	self_type* operator->() noexcept
	{ return this; }
	/// helper to support iterator->get<...>() idiom
	const self_type* operator->() const noexcept
	{ return this; }

	/// move to next element (pre-increment)
	self_type& operator++() noexcept
	{ ++m_index; return *this; }
	/// move to previous element (pre-decrement)
	self_type& operator--() noexcept
	{ --m_index; return *this; }
	/// move to next element (post-increment)
	self_type operator++(int) noexcept
	{ self_type retVal(*this); operator++(); return retVal; }
	/// move to previous element (post-decrement)
	self_type operator--(int) noexcept
	{ self_type retVal(*this); operator--(); return retVal; }

	/// advance dist elements
	self_type& operator+=(difference_type dist) noexcept
	{ m_index += dist; return *this; }
	/// retreat dist elements
	self_type& operator-=(difference_type dist) noexcept
	{ m_index -= dist; return *this; }
	/// advance dist elements
	self_type operator+(difference_type dist) const noexcept
	{ return self_type(*this) += dist; }
	/// retreat dist elements
	self_type operator-(difference_type dist) const noexcept
	{ return self_type(*this) -= dist; }
	/// distance between two self_type instances
	difference_type operator-(const self_type& other) const noexcept
	{
	    return m_storage == other.m_storage ?
		(m_index - other.m_index) : -1;
	}

	/// check if self_type is valid (i.e. points to something)
	operator bool() const noexcept
	{
	    return nullptr != m_storage &&
		m_index < std::get<0>(*m_storage).size();
	}

	template <typename T>
	friend bool operator==(const SOAObjectProxy<T>&, const SOAObjectProxy<T>&) noexcept;
	template <typename T>
	friend bool operator!=(const SOAObjectProxy<T>&, const SOAObjectProxy<T>&) noexcept;
	template <typename T>
	friend bool operator<(const SOAObjectProxy<T>&, const SOAObjectProxy<T>&) noexcept;
	template <typename T>
	friend bool operator<=(const SOAObjectProxy<T>&, const SOAObjectProxy<T>&) noexcept;
	template <typename T>
	friend bool operator>(const SOAObjectProxy<T>&, const SOAObjectProxy<T>&) noexcept;
	template <typename T>
	friend bool operator>=(const SOAObjectProxy<T>&, const SOAObjectProxy<T>&) noexcept;
}; 

/// check for equality (pointer aspect)
template <typename T>
bool operator==(const SOAObjectProxy<T>& a, const SOAObjectProxy<T>& b) noexcept
{ return a.m_storage == b.m_storage && a.m_index == b.m_index; }

/// check for inequality (pointer aspect)
template <typename T>
bool operator!=(const SOAObjectProxy<T>& a, const SOAObjectProxy<T>& b) noexcept
{ return a.m_storage != b.m_storage || a.m_index != b.m_index; }

/// ordering comparison: < (pointer aspect)
template <typename T>
bool operator<(const SOAObjectProxy<T>& a, const SOAObjectProxy<T>& b) noexcept
{
    return (a.m_storage < b.m_storage) ? true :
	(b.m_storage < a.m_storage) ? false :
	a.m_index < b.m_index;
}

/// ordering comparison: <= (pointer aspect)
template <typename T>
bool operator<=(const SOAObjectProxy<T>& a, const SOAObjectProxy<T>& b) noexcept
{
    return (a.m_storage < b.m_storage) ? true :
	(b.m_storage < a.m_storage) ? false :
	a.m_index <= b.m_index;
}

/// ordering comparison: > (pointer aspect)
template <typename T>
bool operator>(const SOAObjectProxy<T>& a, const SOAObjectProxy<T>& b) noexcept
{
    return (a.m_storage < b.m_storage) ? false :
	(b.m_storage < a.m_storage) ? true :
	b.m_index < a.m_index;
}

/// ordering comparison: >= (pointer aspect)
template <typename T>
bool operator>=(const SOAObjectProxy<T>& a, const SOAObjectProxy<T>& b) noexcept
{
    return (a.m_storage < b.m_storage) ? false :
	(b.m_storage < a.m_storage) ? true :
	b.m_index <= a.m_index;
}

/// compare proxy and value (by value)
template <typename T>
bool operator==(const SOAObjectProxy<T>& a, const typename SOAObjectProxy<T>::value_type& b) noexcept
{ return static_cast<typename SOAObjectProxy<T>::value_type>(a) == b; }
/// compare proxy and value (by value)
template <typename T>
bool operator!=(const SOAObjectProxy<T>& a, const typename SOAObjectProxy<T>::value_type& b) noexcept
{ return static_cast<typename SOAObjectProxy<T>::value_type>(a) != b; }
/// compare proxy and value (by value)
template <typename T>
bool operator<(const SOAObjectProxy<T>& a, const typename SOAObjectProxy<T>::value_type& b) noexcept
{ return static_cast<typename SOAObjectProxy<T>::value_type>(a) < b; }
/// compare proxy and value (by value)
template <typename T>
bool operator<=(const SOAObjectProxy<T>& a, const typename SOAObjectProxy<T>::value_type& b) noexcept
{ return static_cast<typename SOAObjectProxy<T>::value_type>(a) <= b; }
/// compare proxy and value (by value)
template <typename T>
bool operator>(const SOAObjectProxy<T>& a, const typename SOAObjectProxy<T>::value_type& b) noexcept
{ return static_cast<typename SOAObjectProxy<T>::value_type>(a) > b; }
/// compare proxy and value (by value)
template <typename T>
bool operator>=(const SOAObjectProxy<T>& a, const typename SOAObjectProxy<T>::value_type& b) noexcept
{ return static_cast<typename SOAObjectProxy<T>::value_type>(a) >= b; }

/// compare proxy and value (by value)
template <typename T>
bool operator==(const typename SOAObjectProxy<T>::value_type& a, const SOAObjectProxy<T>& b) noexcept
{ return a == static_cast<typename SOAObjectProxy<T>::value_type>(b); }
/// compare proxy and value (by value)
template <typename T>
bool operator!=(const typename SOAObjectProxy<T>::value_type& a, const SOAObjectProxy<T>& b) noexcept
{ return a != static_cast<typename SOAObjectProxy<T>::value_type>(b); }
/// compare proxy and value (by value)
template <typename T>
bool operator<(const typename SOAObjectProxy<T>::value_type& a, const SOAObjectProxy<T>& b) noexcept
{ return a < static_cast<typename SOAObjectProxy<T>::value_type>(b); }
/// compare proxy and value (by value)
template <typename T>
bool operator<=(const typename SOAObjectProxy<T>::value_type& a, const SOAObjectProxy<T>& b) noexcept
{ return a <= static_cast<typename SOAObjectProxy<T>::value_type>(b); }
/// compare proxy and value (by value)
template <typename T>
bool operator>(const typename SOAObjectProxy<T>::value_type& a, const SOAObjectProxy<T>& b) noexcept
{ return a > static_cast<typename SOAObjectProxy<T>::value_type>(b); }
/// compare proxy and value (by value)
template <typename T>
bool operator>=(const typename SOAObjectProxy<T>::value_type& a, const SOAObjectProxy<T>& b) noexcept
{ return a >= static_cast<typename SOAObjectProxy<T>::value_type>(b); }

#endif // SOAOBJECTPROXY_H

// vim: sw=4:tw=78:ft=cpp
