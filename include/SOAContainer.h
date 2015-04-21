/** @file SOAContainer.h
 *
 * @author Manuel Schiller <Manuel.Schiller@cern.ch>
 * @date 2015-04-10
 */

#ifndef SOACONTAINER_H
#define SOACONTAINER_H

#include <stdexcept>
#include <algorithm>

#include "SOATypelist.h"
#include "SOATypelistUtils.h"
#include "SOAObjectProxy.h"
#include "SOAIterator.h"
#include "SOAUtils.h"

template <typename NAKEDPROXY>
class NullSkin : public NAKEDPROXY
{
    public:
	template <typename... ARGS>
	NullSkin(ARGS&&... args) : NAKEDPROXY(std::forward<ARGS>(args)...) { }
};

/** @brief container class for objects with given fields (SOA storage)
 *
 * @author Manuel Schiller <Manuel.Schiller@cern.ch>
 * @date 2015-04-10
 *
 * This class represents a container of objects with the given list of fields,
 * the objects are not stored as such, but each of object's fields gets its own
 * storage array, effectively creating a structure-of-arrays (SOA) layout which
 * is advantageous for vectorisation of algorithms. To illustate the SOA
 * layout, first consider the normal array-of-structures (AOS) layout:
 * 
 * @code
 * struct point {
 *     double m_x;
 *     double m_y;
 * };
 * std::vector<point> points; // container of AOS elements
 * // insert some elements
 * // access an element
 * points[0].m_x *= 2.0;
 * @endcode
 * 
 * The memory layout in the example above will be x of element 0, y of element
 * 0, x of element 1, y of element 1, x of element 2, and so on.
 *
 * For the equivalent example in SOA layout, you'd have to do:
 *
 * @code
 * // need to typedef two "tags" by which to identify fields
 * // (if you used two doubles in the SOAContainer template argument list
 * // below, you couldn't distinguish between the two fields, or refer to them
 * // by their symbolic name; the typedef of the anonymous struct below solves
 * // that issue)
 * typedef struct : wrap<double> point_x;
 * typedef struct : wrap<double> point_y;
 *
 * // create container
 * SOAContainer<std::vector, point_x, point_y> points;
 * // insert some elements
 * // access an element
 * points[0].get<point_x>() *= 2.0;
 * @endcode
 * 
 * The code is very similar to the AOS layout example above, but the memory
 * layout is very different. Internally, the container has two std::vectors,
 * one which holds all the x "members" of the point structure, and one which
 * holds all the "y" members.
 *
 * It is important to realise that there's nothing like the struct point in the
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
 * @tparam CONTAINER 	underlying container type (anything that follows
 * 			std::vector's interface is fine)
 * @tparam FIELDS... 	list of "field tags" describing names an types
 * 			of members
 */
template <template <typename...> class CONTAINER, template <typename> class SKIN, typename... FIELDS>
class SOAContainer {
    private:
	/// little helper to verify the FIELDS template parameter
	template <typename... ARGS>
	struct verify_fields;
	/// specialisation for more than one field
	template <typename HEAD, typename... TAIL>
	struct verify_fields<HEAD, TAIL...>
	{
	    enum {
		value = (std::is_pod<HEAD>::value ||
			SOATypelist::typelist_helpers::is_wrapped<
			HEAD>::value) && verify_fields<TAIL...>::value
	    };
	};
	/// specialisation for one field
	template <typename HEAD>
	struct verify_fields<HEAD>
	{
	    enum {
		value = std::is_pod<HEAD>::value ||
		    SOATypelist::typelist_helpers::is_wrapped<HEAD>::value
	    };
	};
    public:
	// storing objects without state doesn't make sense
	static_assert(1 <= sizeof...(FIELDS),
		"need to supply at least one field");
	// make sure fields are either POD or wrapped types
	static_assert(verify_fields<FIELDS...>::value,
		"Fields should be either plain old data (POD) or "
		"wrapped types.");

	/// type to represent sizes and indices
	typedef std::size_t size_type;
	/// type to represent differences of indices
	typedef std::ptrdiff_t difference_type;
	/// type to represent container itself
	typedef SOAContainer<CONTAINER, SKIN, FIELDS...> self_type;
	typedef SOATypelist::typelist<FIELDS...> fields_typelist;

    private:
	/// type of the storage backend
	typedef typename SOATypelist::typelist_to_tuple_of_containers<
	    fields_typelist,
	    SOATypelist::containerify<CONTAINER> >::type SOAStorage;

	/// storage backend
	SOAStorage m_storage;

	/// tuple type used as values
	typedef typename SOATypelist::typelist_to_tuple<
	    fields_typelist>::type value_tuple_type;
	/// tuple type used as reference
	typedef typename SOATypelist::typelist_to_reftuple<
	    fields_typelist>::type reference_tuple_type;
    public:
	friend class SOAObjectProxy<self_type>;
	typedef SOAObjectProxy<self_type> SOAObjectProxy;

	/// (notion of) type of the contained objects
	typedef typename SOAObjectProxy::value_type value_type;
	/// reference to contained objects
	typedef SOAObjectProxy reference_type;
	/// pointer to contained objects
	typedef SOAObjectProxy pointer_type;
	/// reference to contained objects
	typedef const reference_type const_reference_type;
	/// const pointer to contained objects
	typedef const pointer_type const_pointer_type;
	/// iterator type
	typedef SOAIterator<pointer_type> iterator;
	/// const iterator type
	typedef SOAIterator<const_pointer_type> const_iterator;
	/// reverse iterator type
	typedef std::reverse_iterator<iterator> reverse_iterator;
	/// const reverse iterator type
	typedef std::reverse_iterator<const_iterator> const_reverse_iterator;

	/// return if the container is empty
	bool empty() const { return std::get<0>(m_storage).empty(); }
	/// return the size of the container
	size_type size() const { return std::get<0>(m_storage).size(); }

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
	SOAContainer(const self_type& other) : m_storage(other.m_storage) { }
	/// move constructor
	SOAContainer(self_type&& other) :
	    m_storage(std::move(other.m_storage)) { }

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

    private:
	/// little helper for indexing to implement clear()
	struct clearHelper {
	    template <typename T, typename IDX>
	    bool operator()(T& obj, IDX) const
	    { obj.clear(); return true; }
	};

	/// little helper for indexing to implement pop_back()
	struct pop_backHelper {
	    template <typename T, typename IDX>
	    bool operator()(T& obj, IDX) const
	    { obj.pop_back(); return true; }
	};

	/// little helper for indexing to implement shrink_to_fit()
	struct shrink_to_fitHelper {
	    template <typename T, typename IDX>
	    bool operator()(T& obj, IDX) const
	    { obj.shrink_to_fit(); return true; }
	};

	/// little helper for indexing to implement reserve()
	struct reserveHelper {
	    size_type m_sz;
	    reserveHelper(size_type sz) : m_sz(sz) { }
	    template <typename T, typename IDX>
	    bool operator()(T& obj, IDX) const
	    { obj.reserve(m_sz); return true; }
	};

	/// little helper for indexing to implement capacity()
	struct capacityHelper {
	    template <typename T, typename IDX>
	    size_type operator()(T& obj, IDX) const { return obj.capacity(); }
	};

	/// little helper for indexing to implement max_size()
	struct max_sizeHelper {
	    template <typename T, typename IDX>
	    size_type operator()(T& obj, IDX) const { return obj.max_size(); }
	};

    public:
	/// clear the container
	void clear()
	{
	    SOAUtils::recursive_apply_tuple<sizeof...(FIELDS)>()(m_storage,
		    clearHelper(), [] (bool, bool) { return true; }, true);
       	}

	/// pop the last element off the container
	void pop_back()
	{
	    SOAUtils::recursive_apply_tuple<sizeof...(FIELDS)>()(m_storage,
		    pop_backHelper(), [] (bool, bool) { return true; }, true);
       	}

	/// shrink the underlying storage of the container to fit its size
	void shrink_to_fit()
	{
	    SOAUtils::recursive_apply_tuple<sizeof...(FIELDS)>()(m_storage,
		    shrink_to_fitHelper(), [] (bool, bool) {
		    return true; }, true);
       	}

	/// reserve space for at least sz elements
	void reserve(size_type sz)
	{
	    SOAUtils::recursive_apply_tuple<sizeof...(FIELDS)>()(m_storage,
		    reserveHelper(sz), [] (bool, bool) { return true; }, true);
       	}

	/// return capacity of container
	size_type capacity() const
	{
	    return SOAUtils::recursive_apply_tuple<sizeof...(FIELDS)>()(
		    m_storage, capacityHelper(), [] (
			size_type s1, size_type s2) {
		    return std::min(s1, s2); }, size_type(-1));
       	}

	/// return maximal size of container
	size_type max_size() const
	{
	    return SOAUtils::recursive_apply_tuple<sizeof...(FIELDS)>()(
		    m_storage, max_sizeHelper(), [] (
			size_type s1, size_type s2) {
		    return std::min(s1, s2); }, size_type(-1));
       	}

	/// access specified element
	reference_type operator[](size_type idx)
	{ return reference_type(&m_storage, idx); }
	/// access specified element (read access only)
	const_reference_type operator[](size_type idx) const
	{ return const_reference_type(&const_cast<SOAStorage&>(m_storage), idx); }
	/// access specified element with out of bounds checking
	reference_type at(size_type idx)
	{
	    if (idx < size()) return operator[](idx);
	    else throw std::out_of_range("out of bounds");
	}
	/// access specified element with out of bounds checking (read-only)
	const_reference_type at(size_type idx) const
	{
	    if (idx < size()) return operator[](idx);
	    else throw std::out_of_range("out of bounds");
	}

	/// access first element (non-empty container)
	reference_type front() { return operator[](0); }
	/// access first element (non-empty container, read-only)
	const_reference_type front() const { return operator[](0); }
	/// access last element (non-empty container)
	reference_type back() { return operator[](size() - 1); }
	/// access last element (non-empty container, read-only)
	const_reference_type back() const { return operator[](size() - 1); }

	/// iterator pointing to first element
	iterator begin() { return iterator(operator[](0)); }
	/// const iterator pointing to first element
	const_iterator begin() const { return const_iterator(operator[](0)); }
	/// const iterator pointing to first element
	const_iterator cbegin() const { return const_iterator(operator[](0)); }

	/// iterator pointing one element behind the last element
	iterator end() { return iterator(operator[](size())); }
	/// const iterator pointing one element behind the last element
	const_iterator end() const { return const_iterator(operator[](size())); }
	/// const iterator pointing one element behind the last element
	const_iterator cend() const { return const_iterator(operator[](size())); }

	/// iterator pointing to first element in reverse order
	reverse_iterator rbegin() { return reverse_iterator(end()); }
	/// const iterator pointing to first element in reverse order
	const_reverse_iterator rbegin() const { return const_reverse_iterator(end()); }
	/// const iterator pointing to first element in reverse order
	const_reverse_iterator crbegin() const { return const_reverse_iterator(end()); }

	/// iterator pointing one element behind the last element in reverse order
	reverse_iterator rend() { return reverse_iterator(begin()); }
	/// const iterator pointing one element behind the last element in reverse order
	const_reverse_iterator rend() const { return const_reverse_iterator(begin()); }
	/// const iterator pointing one element behind the last element in reverse order
	const_reverse_iterator crend() const { return const_reverse_iterator(begin()); }

    private:
	/// little helper for resize(sz)
	struct resizeHelper {
	    size_type m_sz;
	    resizeHelper(size_type sz) : m_sz(sz) { }
	    template <typename T, typename IDX>
	    bool operator()(T& obj, IDX) const
	    { obj.resize(m_sz); return true; }
	};

	/// little helper for resize(sz, val)
	struct resizeHelper_val {
	    size_type m_sz;
	    const value_type& m_val;
	    resizeHelper_val(size_type sz, const value_type& val) :
		m_sz(sz), m_val(val) { }
	    template <typename T, typename IDX>
	    bool operator()(T& obj, IDX) const
	    { obj.resize(m_sz, std::get<IDX::value>(m_val)); return true; }
	};

	/// little helper for push_back
	struct push_backHelper {
	    const value_type& m_val;
	    push_backHelper(const value_type& val) : m_val(val) { }
	    template <typename T, typename IDX>
	    bool operator()(T& obj, IDX) const
	    { obj.push_back(std::get<IDX::value>(m_val)); return true; }
	};

	/// little helper for push_back (move variant)
	struct push_backHelper_move {
	    value_type&& m_val;
	    push_backHelper_move(value_type&& val) : m_val(std::move(val)) { }
	    template <typename T, typename IDX>
	    bool operator()(T& obj, IDX) const
	    {
		obj.push_back(std::move(std::get<IDX::value>(m_val)));
		return true;
	    }
	};

	/// little helper for insert(it, val)
	struct insertHelper {
	    const value_type& m_val;
	    size_type m_idx;
	    insertHelper(const value_type& val, size_type idx) :
		m_val(val), m_idx(idx) { }
	    template <typename T, typename IDX>
	    bool operator()(T& obj, IDX) const
	    {
		obj.insert(obj.begin() + m_idx, std::get<IDX::value>(m_val));
		return true;
	    }
	};

	/// little helper for insert(it, val) - move variant
	struct insertHelper_move {
	    value_type&& m_val;
	    size_type m_idx;
	    insertHelper_move(value_type&& val, size_type idx) :
		m_val(std::move(val)), m_idx(idx) { }
	    template <typename T, typename IDX>
	    bool operator()(T& obj, IDX) const
	    {
		obj.insert(obj.begin() + m_idx,
			std::move(std::get<IDX::value>(m_val)));
		return true;
	    }
	};

	/// little helper for insert(it, count, val)
	struct insertHelper_count {
	    const value_type& m_val;
	    size_type m_idx;
	    size_type m_cnt;
	    insertHelper_count(
		    const value_type& val, size_type idx, size_type cnt) :
		m_val(val), m_idx(idx), m_cnt(cnt) { }
	    template <typename T, typename IDX>
	    bool operator()(T& obj, IDX) const
	    {
		obj.insert(obj.begin() + m_idx, m_cnt,
			std::get<IDX::value>(m_val));
		return true;
	    }
	};

	/// little helper for erase(it)
	struct eraseHelper {
	    size_type m_idx;
	    eraseHelper(size_type idx) : m_idx(idx) { }
	    template <typename T, typename IDX>
	    bool operator()(T& obj, IDX) const
	    {
		obj.erase(obj.begin() + m_idx);
		return true;
	    }
	};

	/// little helper for erase(first, last)
	struct eraseHelper_N {
	    size_type m_idx;
	    size_type m_sz;
	    eraseHelper_N(size_type idx, size_type sz) :
		m_idx(idx), m_sz(sz) { }
	    template <typename T, typename IDX>
	    bool operator()(T& obj, IDX) const
	    {
		const auto it = obj.begin() + m_idx;
		obj.erase(it, it + m_sz);
		return true;
	    }
	};

	/// little helper for assign(count, val)
	struct assignHelper {
	    const value_type& m_val;
	    size_type m_cnt;
	    assignHelper(
		    const value_type& val, size_type cnt) :
		m_val(val), m_cnt(cnt) { }
	    template <typename T, typename IDX>
	    bool operator()(T& obj, IDX) const
	    {
		obj.assign(m_cnt, std::get<IDX::value>(m_val));
		return true;
	    }
	};

    public:
	/// resize container (use default-constructed values if container grows)
	void resize(size_type sz)
	{
	    SOAUtils::recursive_apply_tuple<sizeof...(FIELDS)>()(m_storage,
		    resizeHelper(sz), [] (bool, bool) {
		    return true; }, true);
	}

	/// resize the container (append val if the container grows)
	void resize(size_type sz, const value_type& val)
	{
	    SOAUtils::recursive_apply_tuple<sizeof...(FIELDS)>()(m_storage,
		    resizeHelper_val(sz, val), [] (bool, bool) {
		    return true; }, true);
	}

	/// push an element at the back of the array
	void push_back(const value_type& val)
	{
	    SOAUtils::recursive_apply_tuple<sizeof...(FIELDS)>()(m_storage,
		    push_backHelper(val), [] (bool, bool) {
		    return true; }, true);
	}

	/// push an element at the back of the array (move variant)
	void push_back(value_type&& val)
	{
	    SOAUtils::recursive_apply_tuple<sizeof...(FIELDS)>()(m_storage,
		    push_backHelper_move(std::move(val)), [] (bool, bool) {
		    return true; }, true);
	}

	/// insert a value at the given position
	iterator insert(const_iterator pos, const value_type& val)
	{
	    assert((*pos).m_storage == &m_storage);
	    const size_type idx = pos - cbegin();
	    SOAUtils::recursive_apply_tuple<sizeof...(FIELDS)>()(m_storage,
		    insertHelper(val, idx), [] (bool, bool) {
		    return true; }, true);
	    return iterator(*pos);
	}

	/// insert a value at the given position (move variant)
	iterator insert(const_iterator pos, value_type&& val)
	{
	    assert((*pos).m_storage == &m_storage);
	    const size_type idx = pos - cbegin();
	    SOAUtils::recursive_apply_tuple<sizeof...(FIELDS)>()(m_storage,
		    insertHelper_move(std::move(val), idx), [] (bool, bool) {
		    return true; }, true);
	    return iterator(*pos);
	}

	/// insert count copies of value at the given position
	iterator insert(
		const_iterator pos, size_type count, const value_type& val)
	{
	    assert((*pos).m_storage == &m_storage);
	    const size_type idx = pos - cbegin();
	    SOAUtils::recursive_apply_tuple<sizeof...(FIELDS)>()(m_storage,
		    insertHelper_count(val, idx, count), [] (bool, bool) {
		    return true; }, true);
	    return iterator(*pos);
	}

	/// insert elements between first and last at position pos
	template <typename IT>
	iterator insert(const_iterator pos, IT first, IT last)
	{
	    iterator retVal(*pos);
	    while (first != last) { insert(pos, *first); ++first; ++pos; }
	    return retVal;
	}

	/// erase an element at the given position
	iterator erase(const_iterator pos)
	{
	    assert((*pos).m_storage == &m_storage);
	    const size_type idx = pos - cbegin();
	    SOAUtils::recursive_apply_tuple<sizeof...(FIELDS)>()(m_storage,
		    eraseHelper(idx), [] (bool, bool) { return true; }, true);
	    return iterator(*pos);
	}

	/// erase elements from first to last
	iterator erase(const_iterator first, const_iterator last)
	{
	    assert((*first).m_storage == &m_storage);
	    assert((*last).m_storage == &m_storage);
	    const size_type idx = first - cbegin();
	    const size_type sz = last - first;
	    SOAUtils::recursive_apply_tuple<sizeof...(FIELDS)>()(m_storage,
		    eraseHelper_N(idx, sz), [] (bool, bool) {
		    return true; }, true);
	    return iterator(*first);
	}

	/// assign the vector to contain count copies of val
	void assign(size_type count, const value_type& val)
	{
	    SOAUtils::recursive_apply_tuple<sizeof...(FIELDS)>()(m_storage,
		    assignHelper(val, count), [] (bool, bool) { return true; }, true);
	}

	/// assign the vector from a range of elements in another container
	template <typename IT>
	void assign(IT first, IT last)
	{
	    if (!empty()) clear();
	    // naively, one would use a reserve(distance(first, last)) here,
	    // but I'm not sure how this will work for various kinds of
	    // special iterators - callers are expected to reserve beforehand
	    // if the required size is known
	    insert(begin(), first, last);
	}

    private:
	/// helper for emplace_back
	template <size_type IDX>
	struct emplace_backHelper {
	    SOAStorage& m_storage;
	    emplace_backHelper(SOAStorage& storage) : m_storage(storage) { }
	    template <typename HEAD, typename... TAIL>
	    void doIt(HEAD&& head, TAIL&&... tail) const
	    {
		std::get<IDX>(m_storage).emplace_back(std::forward<HEAD>(head));
		emplace_backHelper<IDX + 1>(m_storage).doIt(
			std::forward<TAIL>(tail)...);
	    }
	    template <typename HEAD>
	    void doIt(HEAD&& head) const
	    {
		std::get<IDX>(m_storage).emplace_back(std::forward<HEAD>(head));
	    }
	};
	/// helper for emplace
	template <size_type IDX>
	struct emplaceHelper {
	    SOAStorage& m_storage;
	    size_type m_idx;
	    emplaceHelper(SOAStorage& storage, size_type idx) :
		m_storage(storage), m_idx(idx) { }
	    template <typename HEAD, typename... TAIL>
	    void doIt(HEAD&& head, TAIL&&... tail) const
	    {
		std::get<IDX>(m_storage).emplace(
			std::get<IDX>(m_storage).begin() + m_idx,
			std::forward<HEAD>(head));
		emplaceHelper<IDX + 1>(m_storage, m_idx).doIt(
			std::forward<TAIL>(tail)...);
	    }
	    template <typename HEAD>
	    void doIt(HEAD&& head) const
	    {
		std::get<IDX>(m_storage).emplace(
			std::get<IDX>(m_storage).begin() + m_idx,
			std::forward<HEAD>(head));
	    }
	};

    public:
	/// construct new element at end of container (in-place) from args
	template <typename... ARGS>
	void emplace_back(ARGS&&... args)
	{
	    static_assert(sizeof...(ARGS) == sizeof...(FIELDS),
		    "Wrong number of arguments to emplace_back.");
	    emplace_backHelper<0>(m_storage).doIt(
		    std::forward<ARGS>(args)...);
	}
	/// construct new element at position pos (in-place) from args
	template <typename... ARGS>
	iterator emplace(const_iterator pos, ARGS&&... args)
	{
	    static_assert(sizeof...(ARGS) == sizeof...(FIELDS),
		    "Wrong number of arguments to emplace_back.");
	    assert(&m_storage == (*pos).m_storage);
	    emplaceHelper<0>(m_storage, pos - cbegin()).doIt(
		    std::forward<ARGS>(args)...);
	    return iterator(*pos);
	}

};

/// comparison between a SOAObjectProxy::value_type and the proxy
template <typename PROXY>
typename std::enable_if<std::is_same<typename std::remove_const<PROXY>::type,
    typename PROXY::parent_type::reference_type>::value , bool>::type
operator==(const typename PROXY::value_type& a, const PROXY b) noexcept
{ return b == a; }

/// comparison between a SOAObjectProxy::value_type and the proxy
template <typename PROXY>
typename std::enable_if<std::is_same<typename std::remove_const<PROXY>::type,
    typename PROXY::parent_type::reference_type>::value , bool>::type
operator!=(const typename PROXY::value_type& a, const PROXY b) noexcept
{ return b != a; }

/// comparison between a SOAObjectProxy::value_type and the proxy
template <typename PROXY>
typename std::enable_if<std::is_same<typename std::remove_const<PROXY>::type,
    typename PROXY::parent_type::reference_type>::value , bool>::type
operator<(const typename PROXY::value_type& a, const PROXY b) noexcept
{ return b > a; }

/// comparison between a SOAObjectProxy::value_type and the proxy
template <typename PROXY>
typename std::enable_if<std::is_same<typename std::remove_const<PROXY>::type,
    typename PROXY::parent_type::reference_type>::value , bool>::type
operator<=(const typename PROXY::value_type& a, const PROXY b) noexcept
{ return b >= a; }

/// comparison between a SOAObjectProxy::value_type and the proxy
template <typename PROXY>
typename std::enable_if<std::is_same<typename std::remove_const<PROXY>::type,
    typename PROXY::parent_type::reference_type>::value , bool>::type
operator>(const typename PROXY::value_type& a, const PROXY b) noexcept
{ return b < a; }

/// comparison between a SOAObjectProxy::value_type and the proxy
template <typename PROXY>
typename std::enable_if<std::is_same<typename std::remove_const<PROXY>::type,
    typename PROXY::parent_type::reference_type>::value , bool>::type
operator>=(const typename PROXY::value_type& a, const PROXY b) noexcept
{ return b <= a; }

#endif // SOACONTAINER_H

// vim: sw=4:tw=78:ft=cpp
