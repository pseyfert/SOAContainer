/** @file SOAIterator.h
 *
 * @author Manuel Schiller <Manuel.Schiller@cern.ch>
 * @date 2015-10-02
 */

#ifndef SOAITERATOR_H
#define SOAITERATOR_H

#include <iterator>

/** @brief class mimicking a const pointer to pointee inidicated by PROXY
 *
 * @author Manuel Schiller <Manuel.Schiller@cern.ch>
 * @date 2015-05-03
 *
 * @tparam PROXY	proxy class
 */
template <typename PROXY>
class SOAConstIterator
{
    protected:
	PROXY m_proxy; ///< pointee

	// parent container is a friend
	friend typename PROXY::parent_type;

    public:
	/// convenience typedef for our own type
	typedef SOAConstIterator<PROXY> self_type;
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
	typedef SOAConstIterator<PROXY> pointer;
	/// typedef for const pointer
	typedef SOAConstIterator<PROXY> const_pointer;
	/// iterator category
	typedef std::random_access_iterator_tag iterator_category;

	/// copying constructor from proxy
	SOAConstIterator(const PROXY& proxy) noexcept : m_proxy(proxy) { }
	/// moving constructor from proxy
	SOAConstIterator(PROXY&& proxy) noexcept : m_proxy(std::move(proxy)) { }

    protected:
	/// constructor building proxy in place
	SOAConstIterator(
		typename PROXY::parent_type::SOAStorage* storage,
		size_type index) noexcept :
	    m_proxy(storage, index) { }

    public:
	/// copy constructor
	SOAConstIterator(const self_type& other) noexcept :
	    m_proxy(other.m_proxy) { }
	/// move constructor
	SOAConstIterator(self_type&& other) noexcept :
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

/// implement integer + SOAConstIterator
template <typename PROXY, typename T>
typename std::enable_if<
    std::is_integral<T>::value && std::is_convertible<T,
        typename SOAConstIterator<PROXY>::difference_type>::value,
    SOAConstIterator<PROXY> >::type
    operator+(T dist, const SOAConstIterator<PROXY>& other) noexcept
{ return other + dist; }

/** @brief class mimicking a pointer to pointee inidicated by PROXY
 *
 * @author Manuel Schiller <Manuel.Schiller@cern.ch>
 * @date 2015-05-03
 *
 * @tparam PROXY	proxy class
 */
template <typename PROXY>
class SOAIterator : public SOAConstIterator<PROXY>
{
    private:
	// parent container is a friend
	friend typename PROXY::parent_type;

    public:
	/// convenience typedef for our own type
	typedef SOAConstIterator<PROXY> self_type;
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
	typedef SOAIterator<PROXY> pointer;
	/// typedef for const pointer
	typedef SOAConstIterator<PROXY> const_pointer;
	/// iterator category
	typedef std::random_access_iterator_tag iterator_category;

	/// copying constructor from proxy
	SOAIterator(const PROXY& proxy) noexcept :
	    SOAConstIterator<PROXY>(proxy) { }
	/// moving constructor from proxy
	SOAIterator(PROXY&& proxy) noexcept :
	    SOAConstIterator<PROXY>(std::move(proxy)) { }

    private:
	/// constructor building proxy in place
	SOAIterator(
		typename PROXY::parent_type::SOAStorage* storage,
		size_type index) noexcept :
	    SOAConstIterator<PROXY>(storage, index) { }

    public:
	/// copy constructor
	SOAIterator(const self_type& other) noexcept :
	    SOAConstIterator<PROXY>(other) { }

	/// move constructor
	SOAIterator(self_type&& other) noexcept :
	    SOAConstIterator<PROXY>(std::move(other)) { }

	/// assignment
	self_type& operator=(const self_type& other) noexcept
	{ SOAConstIterator<PROXY>::operator=(other); return *this; }
	/// assignment (move semantics)
	self_type& operator=(self_type&& other) noexcept
	{ SOAConstIterator<PROXY>::operator=(std::move(other)); return *this; }

	/// deference pointer (*p)
	reference operator*() noexcept
	{ return SOAConstIterator<PROXY>::m_proxy; }
	/// deference pointer (*p)
	const_reference operator*() const noexcept
	{ return SOAConstIterator<PROXY>::m_proxy; }
	/// deference pointer (p->blah)
	const self_type& operator->() noexcept
	{ return *this; }
	/// deference pointer (p->blah)
	const self_type& operator->() const noexcept
	{ return *this; }

	/// (pre-)increment
	self_type& operator++() noexcept
	{ ++SOAConstIterator<PROXY>::m_proxy.m_index; return *this; }
	/// (pre-)decrement
	self_type& operator--() noexcept
	{ --SOAConstIterator<PROXY>::m_proxy.m_index; return *this; }
	/// (post-)increment
	self_type operator++(int) noexcept
	{
	    self_type retVal(*this);
	    ++SOAConstIterator<PROXY>::m_proxy.m_index;
	    return retVal;
	}
	/// (post-)decrement
	self_type operator--(int) noexcept
	{
	    self_type retVal(*this);
	    --SOAConstIterator<PROXY>::m_proxy.m_index;
	    return retVal;
	}
	/// advance by dist elements
	self_type& operator+=(difference_type dist) noexcept
	{ SOAConstIterator<PROXY>::m_proxy.m_index += dist; return *this; }
	/// "retreat" by dist elements
	self_type& operator-=(difference_type dist) noexcept
	{ SOAConstIterator<PROXY>::m_proxy.m_index -= dist; return *this; }
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
	difference_type operator-(
		const SOAConstIterator<PROXY>& other) const noexcept
	{ return SOAConstIterator<PROXY>::operator-(other); }

	/// indexing
	reference operator[](size_type idx) noexcept
	{ return *((*this) + idx); }
	/// indexing
	const_reference operator[](size_type idx) const noexcept
	{ return *((*this) + idx); }
};

/// implement integer + SOAIterator
template <typename PROXY, typename T>
typename std::enable_if<
    std::is_integral<T>::value && std::is_convertible<T,
        typename SOAIterator<PROXY>::difference_type>::value,
    SOAIterator<PROXY> >::type
    operator+(T dist, const SOAIterator<PROXY>& other) noexcept
{ return other + dist; }

#endif // SOAITERATOR_H

// vim: sw=4:tw=78:ft=cpp
