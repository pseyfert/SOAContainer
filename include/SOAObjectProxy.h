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

template <typename PROXY>
class SOAConstPtr;
template <typename PROXY>
class SOAPtr;

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
	/// typedef to identify the type of a pointer
	typedef SOAPtr<self_type> pointer_type;
	/// typedef to identify the type of a const pointer
	typedef SOAConstPtr<self_type> const_pointer_type;

    protected:
	/// type used by the parent container to hold the SOA data
	typedef typename parent_type::SOAStorage SOAStorage;
	/// typelist of fields
	typedef typename parent_type::fields_typelist fields_typelist;

	size_type m_index;	///< index into underlying SOA storage
	SOAStorage* m_storage;	///< underlying SOA storage of members

	// SOAContainer is allowed to invoke the private constructor
	friend parent_type;
	friend SOAConstPtr<self_type>;
	friend SOAPtr<self_type>;
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

	/// assign from tuple of member contents
	self_type& operator=(const value_type& other)
	{
	    reference_type(*this) = other;
	    return *this;
	}

	/// assign from tuple of member contents (move semantics)
	self_type& operator=(value_type&& other)
	{
	    reference_type(*this) = std::move(other);
	    return *this;
	}

	/// assign from tuple of member contents
	self_type& operator=(const reference_type& other)
	{
	    reference_type(*this) = other;
	    return *this;
	}

	/// assign from tuple of member contents (move semantics)
	self_type& operator=(reference_type&& other)
	{
	    reference_type(*this) = std::move(other);
	    return *this;
	}

	/// assignment operator (value semanticis)
	self_type& operator=(const self_type& other)
	{
	    if (&other != this)
		*this = reference_type(other);
	    return *this;
	}

	/// move assignment operator (value semantics)
	self_type& operator=(self_type&& other)
	{
	    if (&other != this)
		*this = std::move(reference_type(other));
	    return *this;
	}

	/// assignment (pointer-like semantics)
	self_type& assign(const self_type& other)
	{
	    if (&other != this)
		m_index = other.m_index, m_storage = other.m_storage;
	    return *this;
	}
	/// move assignment (pointer-like semantics)
	self_type& assign(self_type&& other)
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
	operator reference_type()
	{
	    return SOAUtils::recursive_apply_tuple<
		fields_typelist::size()>()(
		    *m_storage, to_referenceHelper({ m_index }),
		    tuplecatHelper(), std::tuple<>());
	}

	/// "deep"-swap the contents of two SOAObjectProxy instances
	void swap(self_type& other)
	{ std::swap(reference_type(*this), reference_type(other)); }

	bool operator==(const value_type& other) const noexcept
	{ return value_type(*this) == other; }
	bool operator!=(const value_type& other) const noexcept
	{ return value_type(*this) != other; }
	bool operator<(const value_type& other) const noexcept
	{ return value_type(*this) < other; }
	bool operator>(const value_type& other) const noexcept
	{ return value_type(*this) > other; }
	bool operator<=(const value_type& other) const noexcept
	{ return value_type(*this) <= other; }
	bool operator>=(const value_type& other) const noexcept
	{ return value_type(*this) >= other; }

	bool operator==(const self_type& other) const noexcept
	{ return *this == value_type(other); }
	bool operator!=(const self_type& other) const noexcept
	{ return *this != value_type(other); }
	bool operator<(const self_type& other) const noexcept
	{ return *this < value_type(other); }
	bool operator>(const self_type& other) const noexcept
	{ return *this > value_type(other); }
	bool operator<=(const self_type& other) const noexcept
	{ return *this <= value_type(other); }
	bool operator>=(const self_type& other) const noexcept
	{ return *this >= value_type(other); }
};

template <typename T>
bool operator==(const typename SOAObjectProxy<T>::value_type& a,
	const SOAObjectProxy<T>& b) noexcept { return b == a; }
template <typename T>
bool operator!=(const typename SOAObjectProxy<T>::value_type& a,
	const SOAObjectProxy<T>& b) noexcept { return b != a; }
template <typename T>
bool operator<(const typename SOAObjectProxy<T>::value_type& a,
	const SOAObjectProxy<T>& b) noexcept { return b > a; }
template <typename T>
bool operator>(const typename SOAObjectProxy<T>::value_type& a,
	const SOAObjectProxy<T>& b) noexcept { return b < a; }
template <typename T>
bool operator<=(const typename SOAObjectProxy<T>::value_type& a,
	const SOAObjectProxy<T>& b) noexcept { return b >= a; }
template <typename T>
bool operator>=(const typename SOAObjectProxy<T>::value_type& a,
	const SOAObjectProxy<T>& b) noexcept { return b <= a; }

/** @brief class mimicking a const pointer to pointee inidicated by PROXY
 *
 * @author Manuel Schiller <Manuel.Schiller@cern.ch>
 * @date 2015-05-03
 *
 * @tparam PROXY	proxy class
 */
template <typename PROXY>
class SOAConstPtr
{
    protected:
	PROXY m_proxy; ///< pointee

	friend typename PROXY::parent_type;

    public:
	/// convenience typedef for our own type
	typedef SOAConstPtr<PROXY> self_type;
	/// import value_type from PROXY
	typedef const PROXY value_type;
	/// import size_type from PROXY
	typedef typename PROXY::size_type size_type;
	/// import difference_type from PROXY
	typedef typename PROXY::difference_type difference_type;
	/// typedef for reference to pointee
	typedef const PROXY reference;
	/// typedef for const reference to pointee
	typedef const PROXY const_reference;
	/// typedef for pointer
	typedef SOAConstPtr<PROXY> pointer;
	/// typedef for const pointer
	typedef SOAConstPtr<PROXY> const_pointer;
	/// iterator category
	typedef std::random_access_iterator_tag iterator_category;

	/// copying constructor from proxy
	SOAConstPtr(const PROXY& proxy) noexcept : m_proxy(proxy) { }
	/// moving constructor from proxy
	SOAConstPtr(PROXY&& proxy) noexcept : m_proxy(std::move(proxy)) { }
    protected:
	/// constructor building proxy in place
	SOAConstPtr(typename PROXY::parent_type::SOAStorage* storage, size_type index) noexcept :
	    m_proxy(storage, index) { }

    public:
	/// copy constructor
	SOAConstPtr(const self_type& other) noexcept :
	    m_proxy(other.m_proxy) { }
	/// move constructor
	SOAConstPtr(self_type&& other) noexcept :
	    m_proxy(std::move(other.m_proxy)) { }

	/// assignment
	self_type& operator=(const self_type& other) noexcept
	{ if (&other != this) m_proxy = other.m_proxy; return *this; }
	/// assignment (move semantics)
	self_type& operator=(self_type&& other) noexcept
	{
	    if (&other != this) m_proxy = std::move(other.m_proxy);
	    return *this;
	}

	/// deference pointer (*p)
	reference operator*() noexcept
	{ return m_proxy; }
	/// deference pointer (*p)
	const_reference operator*() const noexcept
	{ return m_proxy; }
	/// deference pointer (p->blah)
	const self_type& operator->() noexcept
	{ return *this; }
	/// deference pointer (p->blah)
	const self_type& operator->() const noexcept
	{ return *this; }

	/// (pre-)increment
	self_type& operator++() noexcept
	{ ++m_proxy.m_index; return *this; }
	/// (pre-)decrement
	self_type& operator--() noexcept
	{ --m_proxy.m_index; return *this; }
	/// (post-)increment
	self_type operator++(int) noexcept
	{ self_type retVal(*this); ++m_proxy.m_index; return retVal; }
	/// (post-)decrement
	self_type operator--(int) noexcept
	{ self_type retVal(*this); --m_proxy.m_index; return retVal; }
	/// advance by dist elements
	self_type& operator+=(difference_type dist) noexcept
	{ m_proxy.m_index += dist; return *this; }
	/// "retreat" by dist elements
	self_type& operator-=(difference_type dist) noexcept
	{ m_proxy.m_index -= dist; return *this; }
	/// advance by dist elements
	template <typename T>
	typename std::enable_if<
		std::is_integral<T>::value &&
		std::is_convertible<T, difference_type>::value, self_type&
		>::type operator+=(T dist) noexcept
	{ return (*this += difference_type(dist)); }
	/// "retreat" by dist elements
	template <typename T>
	typename std::enable_if<
		std::is_integral<T>::value &&
		std::is_convertible<T, difference_type>::value, self_type&
		>::type operator-=(T dist) noexcept
	{ return (*this -= difference_type(dist)); }
	/// advance by dist elements
	self_type operator+(difference_type dist) const noexcept
	{ return self_type(*this) += dist; }
	/// "retreat" by dist elements
	self_type operator-(difference_type dist) const noexcept
	{ return self_type(*this) -= dist; }
	/// advance by dist elements
	template <typename T>
	typename std::enable_if<
		std::is_integral<T>::value &&
		std::is_convertible<T, difference_type>::value, self_type
		>::type operator+(T dist) const noexcept
	{ return self_type(*this) += dist; }
	/// "retreat" by dist elements
	template <typename T>
	typename std::enable_if<
		std::is_integral<T>::value &&
		std::is_convertible<T, difference_type>::value, self_type
		>::type operator-(T dist) const noexcept
	{ return self_type(*this) -= dist; }
	/// distance between two pointers
	difference_type operator-(const self_type& other) const noexcept
	{
	    // give warning about buggy code subtracting pointers from
	    // different containers (ill-defined operation on this pointer
	    // class), use plain C style assert here
	    assert(m_proxy.m_storage &&
		    m_proxy.m_storage == other.m_proxy.m_storage);
#if !defined(BREAKACTIVELY) && !defined(NDEBUG)
	    return (m_proxy.m_index - other.m_proxy.m_index);
#else
	    // return distance if pointers from same container, else return
	    // ridiculously large value in the hopes of badly breaking
	    // ill-behaved client code (when asserts are disabled)
	    return (m_proxy.m_storage &&
		    m_proxy.m_storage == other.m_proxy.m_storage) ?
		std::numeric_limits<difference_type>::max :
		(m_proxy.m_index - other.m_proxy.m_index);
#endif
	}

	/// indexing
	reference operator[](size_type idx) noexcept
	{ return *((*this) + idx); }
	/// indexing
	const_reference operator[](size_type idx) const noexcept
	{ return *((*this) + idx); }

	/// comparison (equality)
	bool operator==(const self_type& other) const noexcept
	{
	    return m_proxy.m_storage == other.m_proxy.m_storage &&
		m_proxy.m_index == other.m_proxy.m_index;
	}
	/// comparison (inequality)
	bool operator!=(const self_type& other) const noexcept
	{
	    return m_proxy.m_index != other.m_proxy.m_index ||
		m_proxy.m_storage != other.m_proxy.m_storage;
	}
	/// comparison (less than)
	bool operator<(const self_type& other) const noexcept
	{
	    return m_proxy.m_storage < other.m_proxy.m_storage ? true :
		other.m_proxy.m_storage < m_proxy.m_storage ? false :
		m_proxy.m_index < other.m_proxy.m_index;
	}
	/// comparison (greater than)
	bool operator>(const self_type& other) const noexcept
	{
	    return m_proxy.m_storage < other.m_proxy.m_storage ? false :
		other.m_proxy.m_storage < m_proxy.m_storage ? true :
		other.m_proxy.m_index < m_proxy.m_index;
	}
	/// comparison (less than or equal to)
	bool operator<=(const self_type& other) const noexcept
	{
	    return m_proxy.m_storage < other.m_proxy.m_storage ? true :
		other.m_proxy.m_storage < m_proxy.m_storage ? false :
		m_proxy.m_index <= other.m_proxy.m_index;
	}
	/// comparison (greater than or equal to)
	bool operator>=(const self_type& other) const noexcept
	{
	    return m_proxy.m_storage < other.m_proxy.m_storage ? false :
		other.m_proxy.m_storage < m_proxy.m_storage ? true :
		other.m_proxy.m_index <= m_proxy.m_index;
	}
	/// check for validity (if (ptr) or if (!ptr) idiom)
	operator bool() const noexcept
	{
	    return m_proxy.m_storage &&
		m_proxy.m_index < std::get<0>(*m_proxy.m_storage).size();
	}
};

/// implement integer + SOAConstPtr
template <typename PROXY, typename T>
typename std::enable_if<
	std::is_integral<T>::value && std::is_convertible<T,
	typename SOAConstPtr<PROXY>::difference_type>::value,
	SOAConstPtr<PROXY> >::type
       operator+(T dist, const SOAConstPtr<PROXY>& other) noexcept
{ return other + dist; }

/** @brief class mimicking a pointer to pointee inidicated by PROXY
 *
 * @author Manuel Schiller <Manuel.Schiller@cern.ch>
 * @date 2015-05-03
 *
 * @tparam PROXY	proxy class
 */
template <typename PROXY>
class SOAPtr : public SOAConstPtr<PROXY>
{
    private:
	friend typename PROXY::parent_type;

    public:
	/// convenience typedef for our own type
	typedef SOAConstPtr<PROXY> self_type;
	/// import value_type from PROXY
	typedef PROXY value_type;
	/// import size_type from PROXY
	typedef typename PROXY::size_type size_type;
	/// import difference_type from PROXY
	typedef typename PROXY::difference_type difference_type;
	/// typedef for reference to pointee
	typedef PROXY reference;
	/// typedef for const reference to pointee
	typedef const PROXY const_reference;
	/// typedef for pointer
	typedef SOAPtr<PROXY> pointer;
	/// typedef for const pointer
	typedef SOAConstPtr<PROXY> const_pointer;
	/// iterator category
	typedef std::random_access_iterator_tag iterator_category;

	/// copying constructor from proxy
	SOAPtr(const PROXY& proxy) noexcept : SOAConstPtr<PROXY>(proxy) { }
	/// moving constructor from proxy
	SOAPtr(PROXY&& proxy) noexcept : SOAConstPtr<PROXY>(std::move(proxy)) { }
    private:
	/// constructor building proxy in place
	SOAPtr(typename PROXY::parent_type::SOAStorage* storage, size_type index) noexcept :
	    SOAConstPtr<PROXY>(storage, index) { }

    public:
	/// copy constructor
	SOAPtr(const self_type& other) noexcept :
	    SOAConstPtr<PROXY>(other) { }

	/// move constructor
	SOAPtr(self_type&& other) noexcept :
	    SOAConstPtr<PROXY>(std::move(other)) { }

	/// assignment
	self_type& operator=(const self_type& other) noexcept
	{ SOAConstPtr<PROXY>::operator=(other); return *this; }
	/// assignment (move semantics)
	self_type& operator=(self_type&& other) noexcept
	{ SOAConstPtr<PROXY>::operator=(std::move(other)); return *this; }

	/// deference pointer (*p)
	reference operator*() noexcept
	{ return SOAConstPtr<PROXY>::m_proxy; }
	/// deference pointer (*p)
	const_reference operator*() const noexcept
	{ return SOAConstPtr<PROXY>::m_proxy; }
	/// deference pointer (p->blah)
	const self_type& operator->() noexcept
	{ return *this; }
	/// deference pointer (p->blah)
	const self_type& operator->() const noexcept
	{ return *this; }

	/// (pre-)increment
	self_type& operator++() noexcept
	{ ++SOAConstPtr<PROXY>::m_proxy.m_index; return *this; }
	/// (pre-)decrement
	self_type& operator--() noexcept
	{ --SOAConstPtr<PROXY>::m_proxy.m_index; return *this; }
	/// (post-)increment
	self_type operator++(int) noexcept
	{ self_type retVal(*this); ++SOAConstPtr<PROXY>::m_proxy.m_index; return retVal; }
	/// (post-)decrement
	self_type operator--(int) noexcept
	{ self_type retVal(*this); --SOAConstPtr<PROXY>::m_proxy.m_index; return retVal; }
	/// advance by dist elements
	self_type& operator+=(difference_type dist) noexcept
	{ SOAConstPtr<PROXY>::m_proxy.m_index += dist; return *this; }
	/// "retreat" by dist elements
	self_type& operator-=(difference_type dist) noexcept
	{ SOAConstPtr<PROXY>::m_proxy.m_index -= dist; return *this; }
	/// advance by dist elements
	template <typename T>
	typename std::enable_if<
		std::is_integral<T>::value &&
		std::is_convertible<T, difference_type>::value, self_type
		>::type operator+=(T dist) noexcept
	{ return (*this += difference_type(dist)); }
	/// "retreat" by dist elements
	template <typename T>
	typename std::enable_if<
		std::is_integral<T>::value &&
		std::is_convertible<T, difference_type>::value, self_type
		>::type operator-=(T dist) noexcept
	{ return (*this -= difference_type(dist)); }
	/// advance by dist elements
	self_type operator+(difference_type dist) const noexcept
	{ return self_type(*this) += dist; }
	/// "retreat" by dist elements
	self_type operator-(difference_type dist) const noexcept
	{ return self_type(*this) -= dist; }
	/// advance by dist elements
	template <typename T>
	typename std::enable_if<
		std::is_integral<T>::value &&
		std::is_convertible<T, difference_type>::value, self_type
		>::type operator+(T dist) const noexcept
	{ return self_type(*this) += dist; }
	/// "retreat" by dist elements
	template <typename T>
	typename std::enable_if<
		std::is_integral<T>::value &&
		std::is_convertible<T, difference_type>::value, self_type
		>::type operator-(T dist) const noexcept
	{ return self_type(*this) -= dist; }
	/// return distance between two pointers
	difference_type operator-(const SOAConstPtr<PROXY>& other) const noexcept
	{ return SOAConstPtr<PROXY>::operator-(other); }

	/// indexing
	reference operator[](size_type idx) noexcept
	{ return *((*this) + idx); }
	/// indexing
	const_reference operator[](size_type idx) const noexcept
	{ return *((*this) + idx); }
};

/// implement integer + SOAPtr
template <typename PROXY, typename T>
typename std::enable_if<
	std::is_integral<T>::value && std::is_convertible<T,
	typename SOAPtr<PROXY>::difference_type>::value,
	SOAPtr<PROXY> >::type
       operator+(T dist, const SOAPtr<PROXY>& other) noexcept
{ return other + dist; }

#endif // SOAOBJECTPROXY_H

// vim: sw=4:tw=78:ft=cpp
