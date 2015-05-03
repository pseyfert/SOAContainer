/** @file SOAIterator.h
 *
 * @author Manuel Schiller <Manuel.Schiller@cern.ch>
 * @date 2015-10-02
 */

#ifndef  SOAITERATOR_H
#define SOAITERATOR_H

#include <iterator>

/** @brief iterator class for various SOAContainer instances
 *
 * @author Manuel Schiller <Manuel.Schiller@cern.ch>
 * @date 2015-04-10
 *
 * @tparam T	a pointer type of SOAContainer in question
 *
 * This is a pretty standard iterator class implementation, with the caveats
 * stated in SOAContainer::SOAObjectProxy.
 */
template <typename T>
class SOAIterator : public std::iterator<
		    std::random_access_iterator_tag,
		    typename T::value_type, typename T::difference_type,
		    T, typename T::reference_type>
{
    private:
	T m_base; ///< proxy for underlying object

	friend class SOAIterator<SOAPtr<
	    typename std::remove_const<typename T::reference_type>::type> >;
	friend class SOAIterator<SOAConstPtr<
	    typename std::remove_const<typename T::reference_type>::type> >;
	friend typename T::reference_type::parent_type;

    public:
	/// shorthand to refer to the type of this class
	typedef SOAIterator<T> self_type;
	/// shorthand to refer to corresponding const iterator
	typedef SOAIterator<SOAConstPtr<typename std::remove_const<typename T::reference_type>::type> > self_type_c;

	/// constructor
	SOAIterator(const T& ptr) noexcept : m_base(ptr) { }
	/// move constructor
	SOAIterator(T&& ptr) noexcept : m_base(std::move(ptr)) { }

	/// copy constructor from SOAIterator<S>
	template <typename S>
	SOAIterator(const typename std::enable_if<
		((std::is_const<typename T::reference_type>::value == std::is_const<typename S::reference_type>::value) ||
		 (std::is_const<typename T::reference_type>::value && !std::is_const<typename S::reference_type>::value)) &&
		std::is_same<typename S::const_reference_type,
		typename T::const_reference_type>::value,
		SOAIterator<S> >::type& other) : m_base(other.m_base) { }
	/// move constructor from SOAIterator<S>
	template <typename S>
	SOAIterator(typename std::enable_if<
		((std::is_const<typename T::reference_type>::value == std::is_const<typename S::reference_type>::value) ||
		 (std::is_const<typename T::reference_type>::value && !std::is_const<typename S::reference_type>::value)) &&
		std::is_same<typename S::const_reference_type,
		typename T::const_reference_type>::value,
		SOAIterator<S> >::type&& other) :
	    m_base(std::move(other.m_base))
        { }

    private:
	SOAIterator(typename T::reference_type::parent_type::SOAStorage* storage, typename T::size_type index) :
	    m_base(storage, index) { }

    public:
	/// assignment from T
	self_type& operator=(const T& ptr) noexcept
	{ reinterpret_cast<T>(m_base) = ptr; return *this; }
	/// assignment from T (move semantics)
	self_type& operator=(T&& ptr) noexcept
	{ reinterpret_cast<T>(m_base) = std::move(ptr); return *this; }

	/// assignment from SOAIterator<S>
	template <typename S>
	self_type& operator=(const typename std::enable_if<
		((std::is_const<T>::value == std::is_const<S>::value) ||
		 (std::is_const<T>::value && !std::is_const<S>::value)) &&
		std::is_same<typename std::remove_const<S>::type,
		typename std::remove_const<T>::type>::value,
		SOAIterator<S> >::type& other)
	{
	    if (&other != this) m_base = other.m_base;
	    return *this;
	}
	/// move assignment from SOAIterator<S>
	template <typename S>
	self_type& operator=(typename std::enable_if<
		((std::is_const<T>::value == std::is_const<S>::value) ||
		 (std::is_const<T>::value && !std::is_const<S>::value)) &&
		std::is_same<typename std::remove_const<S>::type,
		typename std::remove_const<T>::type>::value,
		SOAIterator<S> >::type&& other)
	{
	    if (&other != this) m_base = std::move(other.m_base);
	    return *this;
	}

	/// we can always convert to the const_iterator
	operator self_type_c() const { return m_base; }

	/// dereference
	typename T::reference_type operator*() { return *m_base; }
	/// deference (read-only version)
	typename T::const_reference_type operator*() const { return *m_base; }
	/// dereference
	typename T::pointer_type operator->() { return m_base; }
	/// deference (read-only version)
	typename T::const_pointer_type operator->() const { return m_base; }

	/// for stunts like "if (it) *it = ...;"
	operator bool() const noexcept { return bool(m_base); }
	/// pre-increment
	self_type& operator++() noexcept
	{ ++m_base; return *this; }
	/// post-increment
	self_type operator++(int) noexcept
	{ self_type retVal(*this); ++m_base; return retVal; }
	/// pre-decrement
	self_type& operator--() noexcept
	{ --m_base; return *this; }
	/// post-decrement
	self_type operator--(int) noexcept
	{ self_type retVal(*this); --m_base; return retVal; }
	/// addition and assignment
	self_type& operator+=(typename T::difference_type dist) noexcept
	{ m_base += dist; return *this; }
	/// subtraction and assignment
	self_type& operator-=(typename T::difference_type dist) noexcept
	{ m_base -= dist; return *this; }
	/// addition
	self_type operator+(typename T::difference_type dist) const noexcept
	{ return self_type(*this) += dist; }
	/// subtraction
	self_type operator-(typename T::difference_type dist) const noexcept
	{ return self_type(*this) -= dist; }
	/// distance between iterators
	typename T::difference_type operator-(const self_type& other) const noexcept
	{ return m_base - other.m_base; }
	/// addition and assignment
	template <typename S>
	typename std::enable_if<std::is_integral<S>::value &&
	    std::is_convertible<S, typename T::difference_type>::value,
	    self_type&>::type operator+=(S dist) noexcept
        { return (*this += static_cast<typename T::difference_type>(dist)); }
	/// subtraction and assignment
	template <typename S>
	typename std::enable_if<std::is_integral<S>::value &&
	    std::is_convertible<S, typename T::difference_type>::value,
	    self_type&>::type operator-=(S dist) noexcept
        { return (*this -= static_cast<typename T::difference_type>(dist)); }
	/// addition
	template <typename S>
	typename std::enable_if<std::is_integral<S>::value &&
	    std::is_convertible<S, typename T::difference_type>::value,
	    self_type>::type operator+(S dist) const noexcept
	{ return (self_type(*this) +=
		static_cast<typename T::difference_type>(dist)); }
	/// subtraction
	template <typename S>
	typename std::enable_if<std::is_integral<S>::value &&
	    std::is_convertible<S, typename T::difference_type>::value,
	    self_type>::type operator-(S dist) const noexcept
	{ return (self_type(*this) -=
		static_cast<typename T::difference_type>(dist)); }

};

/// addition of constant to iterator (e.g. 3 + it)
template <typename I, typename T>
SOAIterator<T> operator+(I dist, const SOAIterator<T>& other) noexcept
{
    static_assert(std::is_integral<I>::value, "dist must be integer type");
    return SOAIterator<typename std::remove_const<T>::type>(*other) += dist;
}

/// distance between iterators, i.e. it - jt
template <typename S, typename T>
typename S::difference_type operator-(
	const SOAIterator<S>& it, const SOAIterator<T>& jt) noexcept
{
    static_assert(std::is_same<typename S::const_reference_type, typename T::const_reference_type>::value,
	    "SOAIterator<S> - SOAIterator<T> can only be evaluated if "
	    "S and T refer to the same type, up to constness.");
    return *it - *jt;
}

/// comparison of iterators, i.e. it == jt
template <typename S, typename T>
bool operator==(const SOAIterator<S>& it, const SOAIterator<T>& jt) noexcept
{
    static_assert(std::is_same<typename S::const_reference_type, typename T::const_reference_type>::value,
	    "SOAIterator<S> == SOAIterator<T> can only be evaluated if "
	    "S and T refer to the same type, up to constness.");
    return *it == *jt;
}

/// comparison of iterators, i.e. it != jt
template <typename S, typename T>
bool operator!=(const SOAIterator<S>& it, const SOAIterator<T>& jt) noexcept
{
    static_assert(std::is_same<typename S::const_reference_type, typename T::const_reference_type>::value,
	    "SOAIterator<S> != SOAIterator<T> can only be evaluated if "
	    "S and T refer to the same type, up to constness.");
    return *it != *jt;
}

/// comparison of iterators, i.e. it < jt
template <typename S, typename T>
bool operator<(const SOAIterator<S>& it, const SOAIterator<T>& jt) noexcept
{
    static_assert(std::is_same<typename S::const_reference_type, typename T::const_reference_type>::value,
	    "SOAIterator<S> < SOAIterator<T> can only be evaluated if "
	    "S and T refer to the same type, up to constness.");
    return *it < *jt;
}

/// comparison of iterators, i.e. it > jt
template <typename S, typename T>
bool operator>(const SOAIterator<S>& it, const SOAIterator<T>& jt) noexcept
{
    static_assert(std::is_same<typename S::const_reference_type, typename T::const_reference_type>::value,
	    "SOAIterator<S> > SOAIterator<T> can only be evaluated if "
	    "S and T refer to the same type, up to constness.");
    return *it > *jt;
}

/// comparison of iterators, i.e. it <= jt
template <typename S, typename T>
bool operator<=(const SOAIterator<S>& it, const SOAIterator<T>& jt) noexcept
{
    static_assert(std::is_same<typename S::const_reference_type, typename T::const_reference_type>::value,
	    "SOAIterator<S> <= SOAIterator<T> can only be evaluated if "
	    "S and T refer to the same type, up to constness.");
    return *it <= *jt;
}

/// comparison of iterators, i.e. it >= jt
template <typename S, typename T>
bool operator>=(const SOAIterator<S>& it, const SOAIterator<T>& jt) noexcept
{
    static_assert(std::is_same<typename S::const_reference_type, typename T::const_reference_type>::value,
	    "SOAIterator<S> >= SOAIterator<T> can only be evaluated if "
	    "S and T refer to the same type, up to constness.");
    return *it >= *jt;
}

#endif // SOAITERATOR_H

// vim: sw=4:tw=78:ft=cpp
