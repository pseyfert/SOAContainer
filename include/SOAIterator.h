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
 * @tparam T	SOAObjectProxy of SOAContainer in question
 *
 * This is a pretty standard iterator class implementation, with the caveats
 * stated in SOAContainer::SOAObjectProxy.
 */
template <typename T>
class SOAIterator : public std::iterator<
		    std::random_access_iterator_tag,
		    typename T::value_type, typename T::difference_type,
		    T, T>
{
    private:
	T m_base; ///< proxy for underlying object

	friend class SOAIterator<typename std::remove_const<T>::type>;
	friend class SOAIterator<const T>;

    public:
	/// shorthand to refer to the type of this class
	typedef SOAIterator<T> self_type;

	/// constructor
	SOAIterator(const T& ptr) noexcept : m_base(ptr) { }
	/// move constructor
	SOAIterator(T&& ptr) noexcept : m_base(std::move(ptr)) { }

	/// constructor from non-const iterator
	SOAIterator(const SOAIterator<
		typename std::remove_const<T>::type>& other) noexcept :
       	    m_base(other.m_base)
	{ }

	/// move constructor from non-const iterator
	SOAIterator(SOAIterator<
		typename std::remove_const<T>::type>&& other) noexcept :
       	    m_base(std::move(other.m_base))
	{ }

	// copy constructors and move constructors from self_type
	// areautogenerated!

	self_type& operator=(const T& ptr) noexcept
	{ m_base = ptr; return *this; }
	self_type& operator=(T&& ptr) noexcept
	{ m_base = ptr; return *this; }

	/// dereference (but do not leak non-const handle to m_base)
	T operator*() { return m_base; }
	/// deference (read-only version)
	const T& operator*() const { return m_base; }
	/// dereference (but do not leak non-const handle to m_base)
	T operator->() { return m_base; }
	/// deference (read-only version)
	const T& operator->() const { return m_base; }

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
};

/// addition of constant to iterator (e.g. 3 + it)
template <typename I, typename T>
SOAIterator<T> operator+(I dist, const SOAIterator<T>& other) noexcept
{
    static_assert(std::is_integral<I>::value, "dist must be integer type");
    return SOAIterator<T>(other) += dist;
}

/// distance between iterators, i.e. it - jt
template <typename S, typename T>
typename S::difference_type operator-(
	const SOAIterator<S>& it, const SOAIterator<T>& jt) noexcept
{
    static_assert(std::is_same<const S, const T>::value,
	    "SOAIterator<S> - SOAIterator<T> can only be evaluated if "
	    "S and T refer to the same type, up to constness.");
    return *it - *jt;
}

/// comparison of iterators, i.e. it == jt
template <typename S, typename T>
bool operator==(const SOAIterator<S>& it, const SOAIterator<T>& jt) noexcept
{
    static_assert(std::is_same<const S, const T>::value,
	    "SOAIterator<S> == SOAIterator<T> can only be evaluated if "
	    "S and T refer to the same type, up to constness.");
    return *it == *jt;
}

/// comparison of iterators, i.e. it != jt
template <typename S, typename T>
bool operator!=(const SOAIterator<S>& it, const SOAIterator<T>& jt) noexcept
{
    static_assert(std::is_same<const S, const T>::value,
	    "SOAIterator<S> != SOAIterator<T> can only be evaluated if "
	    "S and T refer to the same type, up to constness.");
    return *it != *jt;
}

/// comparison of iterators, i.e. it < jt
template <typename S, typename T>
bool operator<(const SOAIterator<S>& it, const SOAIterator<T>& jt) noexcept
{
    static_assert(std::is_same<const S, const T>::value,
	    "SOAIterator<S> < SOAIterator<T> can only be evaluated if "
	    "S and T refer to the same type, up to constness.");
    return *it < *jt;
}

/// comparison of iterators, i.e. it > jt
template <typename S, typename T>
bool operator>(const SOAIterator<S>& it, const SOAIterator<T>& jt) noexcept
{
    static_assert(std::is_same<const S, const T>::value,
	    "SOAIterator<S> > SOAIterator<T> can only be evaluated if "
	    "S and T refer to the same type, up to constness.");
    return *it > *jt;
}

/// comparison of iterators, i.e. it <= jt
template <typename S, typename T>
bool operator<=(const SOAIterator<S>& it, const SOAIterator<T>& jt) noexcept
{
    static_assert(std::is_same<const S, const T>::value,
	    "SOAIterator<S> <= SOAIterator<T> can only be evaluated if "
	    "S and T refer to the same type, up to constness.");
    return *it <= *jt;
}

/// comparison of iterators, i.e. it >= jt
template <typename S, typename T>
bool operator>=(const SOAIterator<S>& it, const SOAIterator<T>& jt) noexcept
{
    static_assert(std::is_same<const S, const T>::value,
	    "SOAIterator<S> >= SOAIterator<T> can only be evaluated if "
	    "S and T refer to the same type, up to constness.");
    return *it >= *jt;
}

#endif // SOAITERATOR_H

// vim: sw=4:tw=78:ft=cpp
