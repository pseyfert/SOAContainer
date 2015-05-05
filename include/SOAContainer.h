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

/** @brief skin class for SOAContainer which does nothing and preserved the
 * raw proxy interface.
 *
 * @author Manuel Schiller <Manuel.Schiller@cern.ch>
 * @date 2015-04-21
 */
template <typename NAKEDPROXY>
class NullSkin : public NAKEDPROXY
{
    public:
	/// constructor - forward to underlying proxy
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
 * @tparam CONTAINER 	underlying container type (anything that follows
 * 			std::vector's interface is fine)
 * @tparam SKIN		"skin" to dress the the interface provided by the
 * 			get<fieldtag>() syntax with something more convenient;
 * 			NullSkin leaves the raw interface intact
 * @tparam FIELDS... 	list of "field tags" describing names an types
 * 			of members
 */
template < template <typename...> class CONTAINER,
    template <typename> class SKIN, typename... FIELDS>
class SOAContainer {
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
	    		SOATypelist::typelist_helpers::is_wrapped<
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
			SOATypelist::typelist_helpers::is_wrapped<HEAD>::value
	        };
	    };
	    // make sure fields are either POD or wrapped types
	    static_assert(verify_fields<FIELDS...>::value,
	    	"Fields should be either plain old data (POD) or "
	    	"wrapped types.");
	};

    public:
	/// type to represent sizes and indices
	typedef std::size_t size_type;
	/// type to represent differences of indices
	typedef std::ptrdiff_t difference_type;
	/// type to represent container itself
	typedef SOAContainer<CONTAINER, SKIN, FIELDS...> self_type;
	/// typedef holding a typelist with the given fields
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
	/// tuple type used as const reference
	typedef typename SOATypelist::typelist_to_creftuple<
	    fields_typelist>::type const_reference_tuple_type;

    public:
	/// naked proxy type (to be given a "skin" later)
	typedef SOAObjectProxy<self_type> naked_proxy;
	friend naked_proxy;
	/// type of proxy
	typedef SKIN<naked_proxy> proxy;
	friend proxy;
	/// pointer to contained objects
	typedef SOAIterator<proxy> pointer;
	friend pointer;
	/// iterator type
	typedef pointer iterator;
	/// (notion of) type of the contained objects
	typedef typename proxy::value_type value_type;
	/// reference to contained objects
	typedef proxy reference;
	/// reference to contained objects
	typedef const reference const_reference;
	/// const pointer to contained objects
	typedef SOAConstIterator<proxy> const_pointer;
	friend const_pointer;
	/// const iterator type
	typedef const_pointer const_iterator;
	/// reverse iterator type
	typedef std::reverse_iterator<iterator> reverse_iterator;
	/// const reverse iterator type
	typedef std::reverse_iterator<const_iterator> const_reverse_iterator;

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

	/// return if the container is empty
	bool empty() const noexcept(noexcept(std::get<0>(m_storage).empty()))
	{ return std::get<0>(m_storage).empty(); }
	/// return the size of the container
	size_type size() const noexcept(noexcept(
		    std::get<0>(m_storage).size()))
	{ return std::get<0>(m_storage).size(); }

    private:
	/// little helper for indexing to implement clear()
	struct clearHelper {
	    template <typename T, typename IDX>
	    void operator()(T& obj, IDX) const noexcept(noexcept(obj.clear()))
	    { obj.clear(); }
	};

	/// little helper for indexing to implement pop_back()
	struct pop_backHelper {
	    template <typename T, typename IDX>
	    void operator()(T& obj, IDX) const noexcept(
		    noexcept(obj.pop_back()))
	    { obj.pop_back(); }
	};

	/// little helper for indexing to implement shrink_to_fit()
	struct shrink_to_fitHelper {
	    template <typename T, typename IDX>
	    void operator()(T& obj, IDX) const noexcept(
		    noexcept(obj.shrink_to_fit()))
	    { obj.shrink_to_fit(); }
	};

	/// little helper for indexing to implement reserve()
	struct reserveHelper {
	    size_type m_sz;
	    reserveHelper(size_type sz) noexcept : m_sz(sz) { }
	    template <typename T, typename IDX>
	    void operator()(T& obj, IDX) const noexcept(
		    noexcept(obj.reserve(m_sz)))
	    { obj.reserve(m_sz); }
	};

	/// little helper for indexing to implement capacity()
	struct capacityHelper {
	    template <typename T, typename IDX>
	    size_type operator()(T& obj, IDX) const noexcept(
		    noexcept(obj.capacity()))
	    { return obj.capacity(); }
	};

	/// little helper for indexing to implement max_size()
	struct max_sizeHelper {
	    template <typename T, typename IDX>
	    size_type operator()(T& obj, IDX) const noexcept(
		    noexcept(obj.max_size()))
	    { return obj.max_size(); }
	};

    public:
	/// clear the container
	void clear() noexcept(noexcept(
	    SOAUtils::recursive_apply_tuple<sizeof...(FIELDS)>()(m_storage,
		    clearHelper())))
	{
	    SOAUtils::recursive_apply_tuple<sizeof...(FIELDS)>()(m_storage,
		    clearHelper());
       	}

	/// pop the last element off the container
	void pop_back() noexcept(noexcept(
	    SOAUtils::recursive_apply_tuple<sizeof...(FIELDS)>()(m_storage,
		    pop_backHelper())))
	{
	    SOAUtils::recursive_apply_tuple<sizeof...(FIELDS)>()(m_storage,
		    pop_backHelper());
       	}

	/// shrink the underlying storage of the container to fit its size
	void shrink_to_fit() noexcept(noexcept(
	    SOAUtils::recursive_apply_tuple<sizeof...(FIELDS)>()(m_storage,
		    shrink_to_fitHelper())))
	{
	    SOAUtils::recursive_apply_tuple<sizeof...(FIELDS)>()(m_storage,
		    shrink_to_fitHelper());
       	}

	/// reserve space for at least sz elements
	void reserve(size_type sz) noexcept(noexcept(
	    SOAUtils::recursive_apply_tuple<sizeof...(FIELDS)>()(m_storage,
		    reserveHelper(sz))))
	{
	    SOAUtils::recursive_apply_tuple<sizeof...(FIELDS)>()(m_storage,
		    reserveHelper(sz));
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
	reference operator[](size_type idx) noexcept
	{ return { &m_storage, idx }; }
	/// access specified element (read access only)
	const_reference operator[](size_type idx) const noexcept
	{ return { &const_cast<SOAStorage&>(m_storage), idx }; }
	/// access specified element with out of bounds checking
	reference at(size_type idx)
	{
	    if (idx < size()) return operator[](idx);
	    else throw std::out_of_range("out of bounds");
	}
	/// access specified element with out of bounds checking (read-only)
	const_reference at(size_type idx) const
	{
	    if (idx < size()) return operator[](idx);
	    else throw std::out_of_range("out of bounds");
	}

	/// access first element (non-empty container)
	reference front() noexcept { return operator[](0); }
	/// access first element (non-empty container, read-only)
	const_reference front() const noexcept { return operator[](0); }
	/// access last element (non-empty container)
	reference back() noexcept(noexcept(
		    static_cast<self_type*>(nullptr)->size()))
	{ return operator[](size() - 1); }
	/// access last element (non-empty container, read-only)
	const_reference back() const noexcept(noexcept(
		    static_cast<self_type*>(nullptr)->size()))
	{ return operator[](size() - 1); }

	/// iterator pointing to first element
	iterator begin() noexcept { return { &m_storage, 0 }; }
	/// const iterator pointing to first element
	const_iterator begin() const noexcept
	{ return { const_cast<SOAStorage*>(&m_storage), 0 }; }
	/// const iterator pointing to first element
	const_iterator cbegin() const noexcept { return begin(); }

	/// iterator pointing one element behind the last element
	iterator end() noexcept { return { &m_storage, size() }; }
	/// const iterator pointing one element behind the last element
	const_iterator end() const noexcept
	{ return { const_cast<SOAStorage*>(&m_storage), size() }; }
	/// const iterator pointing one element behind the last element
	const_iterator cend() const noexcept { return end(); }

	/// iterator pointing to first element in reverse order
	reverse_iterator rbegin() noexcept { return reverse_iterator(end()); }
	/// const iterator pointing to first element in reverse order
	const_reverse_iterator rbegin() const noexcept
	{ return const_reverse_iterator(end()); }
	/// const iterator pointing to first element in reverse order
	const_reverse_iterator crbegin() const noexcept { return rbegin(); }

	/// iterator pointing one element behind the last element in reverse order
	reverse_iterator rend() noexcept { return reverse_iterator(begin()); }
	/// const iterator pointing one element behind the last element in reverse order
	const_reverse_iterator rend() const noexcept
	{ return const_reverse_iterator(begin()); }
	/// const iterator pointing one element behind the last element in reverse order
	const_reverse_iterator crend() const noexcept { return rend(); }

    private:
	/// little helper for resize(sz)
	struct resizeHelper {
	    size_type m_sz;
	    resizeHelper(size_type sz) noexcept : m_sz(sz) { }
	    template <typename T, typename IDX>
	    void operator()(T& obj, IDX) const noexcept(
		    noexcept(obj.resize(m_sz)))
	    { obj.resize(m_sz); }
	};

	/// little helper for resize(sz, val)
	struct resizeHelper_val {
	    size_type m_sz;
	    const value_type& m_val;
	    resizeHelper_val(size_type sz, const value_type& val) noexcept :
		m_sz(sz), m_val(val) { }
	    template <typename T, typename IDX>
	    void operator()(T& obj, IDX) const noexcept(noexcept(
			obj.resize(m_sz, std::get<IDX::value>(m_val))))
	    { obj.resize(m_sz, std::get<IDX::value>(m_val)); }
	};

	/// little helper for push_back
	struct push_backHelper {
	    const value_type& m_val;
	    push_backHelper(const value_type& val) noexcept : m_val(val) { }
	    template <typename T, typename IDX>
	    void operator()(T& obj, IDX) const noexcept(noexcept(
			obj.push_back(std::get<IDX::value>(m_val))))
	    { obj.push_back(std::get<IDX::value>(m_val)); }
	};

	/// little helper for push_back (move variant)
	struct push_backHelper_move {
	    value_type&& m_val;
	    push_backHelper_move(value_type&& val) noexcept :
		m_val(std::move(val)) { }
	    template <typename T, typename IDX>
	    void operator()(T& obj, IDX) const noexcept(noexcept(
			obj.push_back(std::move(std::get<IDX::value>(m_val)))))
	    { obj.push_back(std::move(std::get<IDX::value>(m_val))); }
	};

	/// little helper for insert(it, val)
	struct insertHelper {
	    const value_type& m_val;
	    size_type m_idx;
	    insertHelper(const value_type& val, size_type idx) noexcept :
		m_val(val), m_idx(idx) { }
	    template <typename T, typename IDX>
	    void operator()(T& obj, IDX) const noexcept(noexcept(obj.insert(
			    obj.begin() + m_idx, std::get<IDX::value>(m_val))))
	    { obj.insert(obj.begin() + m_idx, std::get<IDX::value>(m_val)); }
	};

	/// little helper for insert(it, val) - move variant
	struct insertHelper_move {
	    value_type&& m_val;
	    size_type m_idx;
	    insertHelper_move(value_type&& val, size_type idx) noexcept :
		m_val(std::move(val)), m_idx(idx) { }
	    template <typename T, typename IDX>
	    void operator()(T& obj, IDX) const noexcept(noexcept(
			obj.insert(obj.begin() + m_idx,
			    std::move(std::get<IDX::value>(m_val)))))
	    { obj.insert(obj.begin() + m_idx,
		    std::move(std::get<IDX::value>(m_val))); }
	};

	/// little helper for insert(it, count, val)
	struct insertHelper_count {
	    const value_type& m_val;
	    size_type m_idx;
	    size_type m_cnt;
	    insertHelper_count( const value_type& val, size_type idx,
		    size_type cnt) noexcept :
		m_val(val), m_idx(idx), m_cnt(cnt) { }
	    template <typename T, typename IDX>
	    void operator()(T& obj, IDX) const noexcept(noexcept(
			obj.insert(obj.begin() + m_idx, m_cnt,
			    std::get<IDX::value>(m_val))))
	    {
		obj.insert(obj.begin() + m_idx, m_cnt,
			std::get<IDX::value>(m_val));
	    }
	};

	/// little helper for erase(it)
	struct eraseHelper {
	    size_type m_idx;
	    eraseHelper(size_type idx) noexcept : m_idx(idx) { }
	    template <typename T, typename IDX>
	    void operator()(T& obj, IDX) const noexcept(noexcept(
			obj.erase(obj.begin() + m_idx)))
	    { obj.erase(obj.begin() + m_idx); }
	};

	/// little helper for erase(first, last)
	struct eraseHelper_N {
	    size_type m_idx;
	    size_type m_sz;
	    eraseHelper_N(size_type idx, size_type sz) noexcept :
		m_idx(idx), m_sz(sz) { }
	    template <typename T, typename IDX>
	    void operator()(T& obj, IDX) const noexcept(noexcept(
			obj.erase(obj.begin() + m_idx,
			    obj.begin() + m_idx + m_sz)))
	    { obj.erase(obj.begin() + m_idx, obj.begin() + m_idx + m_sz); }
	};

	/// little helper for assign(count, val)
	struct assignHelper {
	    const value_type& m_val;
	    size_type m_cnt;
	    assignHelper(
		    const value_type& val, size_type cnt) noexcept :
		m_val(val), m_cnt(cnt) { }
	    template <typename T, typename IDX>
	    void operator()(T& obj, IDX) const noexcept(noexcept(
			obj.assign(m_cnt, std::get<IDX::value>(m_val))))
	    { obj.assign(m_cnt, std::get<IDX::value>(m_val)); }
	};

    public:
	/// resize container (use default-constructed values if container grows)
	void resize(size_type sz) noexcept(noexcept(
		    SOAUtils::recursive_apply_tuple<sizeof...(FIELDS)>()(
			m_storage, resizeHelper(sz))))
	{
	    SOAUtils::recursive_apply_tuple<sizeof...(FIELDS)>()(m_storage,
		    resizeHelper(sz));
	}

	/// resize the container (append val if the container grows)
	void resize(size_type sz, const value_type& val) noexcept(noexcept(
		    SOAUtils::recursive_apply_tuple<sizeof...(FIELDS)>()(
			m_storage, resizeHelper_val(sz, val))))
	{
	    SOAUtils::recursive_apply_tuple<sizeof...(FIELDS)>()(m_storage,
		    resizeHelper_val(sz, val));
	}

	/// push an element at the back of the array
	void push_back(const value_type& val) noexcept(noexcept(
		    SOAUtils::recursive_apply_tuple<sizeof...(FIELDS)>()(
			m_storage, push_backHelper(val))))
	{
	    SOAUtils::recursive_apply_tuple<sizeof...(FIELDS)>()(m_storage,
		    push_backHelper(val));
	}

	/// push an element at the back of the array (move variant)
	void push_back(value_type&& val) noexcept(noexcept(
		    SOAUtils::recursive_apply_tuple<sizeof...(FIELDS)>()(
			m_storage, push_backHelper_move(std::move(val)))))
	{
	    SOAUtils::recursive_apply_tuple<sizeof...(FIELDS)>()(m_storage,
		    push_backHelper_move(std::move(val)));
	}

	/// insert a value at the given position
	iterator insert(const_iterator pos, const value_type& val) noexcept(
		noexcept(SOAUtils::recursive_apply_tuple<
		    sizeof...(FIELDS)>()(m_storage,
		    insertHelper(val, pos.m_proxy.m_index))))
	{
	    assert((*pos).m_storage == &m_storage);
	    SOAUtils::recursive_apply_tuple<sizeof...(FIELDS)>()(m_storage,
		    insertHelper(val, pos.m_proxy.m_index));
	    return { pos.m_proxy.m_storage, pos.m_proxy.m_index };
	}

	/// insert a value at the given position (move variant)
	iterator insert(const_iterator pos, value_type&& val) noexcept(
		noexcept(SOAUtils::recursive_apply_tuple<
		    sizeof...(FIELDS)>()(m_storage,
			insertHelper_move(std::move(val),
			    pos.m_proxy.m_index))))
	{
	    assert((*pos).m_storage == &m_storage);
	    SOAUtils::recursive_apply_tuple<sizeof...(FIELDS)>()(m_storage,
		    insertHelper_move(std::move(val), pos.m_proxy.m_index));
	    return { pos.m_proxy.m_storage, pos.m_proxy.m_index };
	}

	/// insert count copies of value at the given position
	iterator insert(const_iterator pos, size_type count, const
		value_type& val) noexcept(noexcept(
			SOAUtils::recursive_apply_tuple<
			sizeof...(FIELDS)>()(m_storage,
			    insertHelper_count(val, pos.m_proxy.m_index,
				count))))
	{
	    assert((*pos).m_storage == &m_storage);
	    SOAUtils::recursive_apply_tuple<sizeof...(FIELDS)>()(m_storage,
		    insertHelper_count(val, pos.m_proxy.m_index, count));
	    return { pos.m_proxy.m_storage, pos.m_proxy.m_index };
	}

	/// insert elements between first and last at position pos
	template <typename IT>
	iterator insert(const_iterator pos, IT first, IT last)
	{
	    // FIXME: terrible implementation!!!
	    iterator retVal(pos.m_proxy.m_storage, pos.m_proxy.m_index);
	    while (first != last) { insert(pos, *first); ++first; ++pos; }
	    return retVal;
	}

	/// erase an element at the given position
	iterator erase(const_iterator pos) noexcept(noexcept(
		    SOAUtils::recursive_apply_tuple<sizeof...(FIELDS)>()(
			m_storage, eraseHelper(pos.m_proxy.m_index))))
	{
	    assert((*pos).m_storage == &m_storage);
	    SOAUtils::recursive_apply_tuple<sizeof...(FIELDS)>()(m_storage,
		    eraseHelper(pos.m_proxy.m_index));
	    return { pos.m_proxy.m_storage, pos.m_proxy.m_index };
	}

	/// erase elements from first to last
	iterator erase(const_iterator first, const_iterator last) noexcept(
		noexcept(
		    SOAUtils::recursive_apply_tuple<sizeof...(FIELDS)>()(
			m_storage, eraseHelper_N(first.m_proxy.m_index,
			    last.m_proxy.m_index - first.m_proxy.m_index))))
	{
	    assert((*first).m_storage == &m_storage);
	    assert((*last).m_storage == &m_storage);
	    SOAUtils::recursive_apply_tuple<sizeof...(FIELDS)>()(m_storage,
		    eraseHelper_N(first.m_proxy.m_index,
			last.m_proxy.m_index - first.m_proxy.m_index));
	    return { first.m_proxy.m_storage, first.m_proxy.m_index };
	}

	/// assign the vector to contain count copies of val
	void assign(size_type count, const value_type& val) noexcept(noexcept(
		    SOAUtils::recursive_apply_tuple<sizeof...(FIELDS)>()(
			m_storage, assignHelper(val, count))))
	{
	    SOAUtils::recursive_apply_tuple<sizeof...(FIELDS)>()(m_storage,
		    assignHelper(val, count));
	}

	/// assign the vector from a range of elements in another container
	template <typename IT>
	void assign(IT first, IT last) noexcept(
		noexcept(empty()) && noexcept(clear()) &&
	       	noexcept(insert(begin(), first, last)))
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
	    emplace_backHelper(SOAStorage& storage) noexcept :
		m_storage(storage) { }
	    template <typename HEAD, typename... TAIL>
	    void doIt(HEAD&& head, TAIL&&... tail) const noexcept(noexcept(
			std::get<IDX>(m_storage).emplace_back(
			    std::forward<HEAD>(head))) && noexcept(
			emplace_backHelper<IDX + 1>(m_storage).doIt(
			    std::forward<TAIL>(tail)...)))
	    {
		std::get<IDX>(m_storage).emplace_back(std::forward<HEAD>(head));
		emplace_backHelper<IDX + 1>(m_storage).doIt(
			std::forward<TAIL>(tail)...);
	    }
	    template <typename HEAD>
	    void doIt(HEAD&& head) const noexcept(noexcept(
			std::get<IDX>(m_storage).emplace_back(
			    std::forward<HEAD>(head))))
	    {
		std::get<IDX>(m_storage).emplace_back(std::forward<HEAD>(head));
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

    public:
	/// construct new element at end of container (in-place) from args
	template <typename... ARGS>
	void emplace_back(ARGS&&... args) noexcept(noexcept(
		    emplace_backHelper<0>(m_storage).doIt(
			std::forward<ARGS>(args)...)))
	{
	    static_assert(sizeof...(ARGS) == sizeof...(FIELDS),
		    "Wrong number of arguments to emplace_back.");
	    emplace_backHelper<0>(m_storage).doIt(
		    std::forward<ARGS>(args)...);
	}
	/// construct new element at position pos (in-place) from args
	template <typename... ARGS>
	iterator emplace(const_iterator pos, ARGS&&... args) noexcept(
		noexcept(emplaceHelper<0>(m_storage,
			pos.m_proxy.m_index).doIt(
			    std::forward<ARGS>(args)...)))
	{
	    static_assert(sizeof...(ARGS) == sizeof...(FIELDS),
		    "Wrong number of arguments to emplace_back.");
	    assert(&m_storage == (*pos).m_storage);
	    emplaceHelper<0>(m_storage, pos.m_proxy.m_index).doIt(
		    std::forward<ARGS>(args)...);
	    return { pos.m_proxy.m_storage, pos.m_proxy.m_index };
	}

	/// swap contents of two containers
	void swap(self_type& other) noexcept(
		noexcept(std::swap(m_storage, other.m_storage)))
	{ std::swap(m_storage, other.m_storage); }
};

namespace std {
    /// specialise std::swap
    template <template <typename...> class CONTAINER,
	     template <typename> class SKIN,
	     typename... FIELDS>
    void swap(const SOAContainer<CONTAINER, SKIN, FIELDS...>& a,
	    const SOAContainer<CONTAINER, SKIN, FIELDS...>& b) noexcept(
		noexcept(a.swap(b)))
    { a.swap(b); }
}

#endif // SOACONTAINER_H

// vim: sw=4:tw=78:ft=cpp
