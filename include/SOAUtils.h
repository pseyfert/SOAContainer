#include <cstdint>
#include <type_traits>

/// type equivalent to the null pointer (tag to indicate missing type)
struct null_type {};

/// typelist data type
template<typename HEAD = null_type, typename... TAIL>
struct typelist {
    /// head of the typelist
    typedef HEAD head_type;
    /// tail is another typelist
    typedef typelist<TAIL...> tail_types;

    /// return size of the typelist
    static constexpr std::size_t size()
    { return 1 + tail_types::size(); }
};

/// specialisation for empty typelist
template <>
constexpr std::size_t typelist<null_type>::size()
{ return 0; }

// test typelist instantiation and size operations
static_assert(0 == typelist<>::size(),
	"size of empty type list");
static_assert(1 == typelist<int>::size(),
	"size of one element type list");
static_assert(2 == typelist<int, float>::size(),
	"size of two element type list");
static_assert(3 == typelist<int, float, double>::size(),
	"size of three element type list");

/// return type at index idx in typelist TL
template <typename TL, std::size_t idx>
struct at {
    typedef typename at<typename TL::tail_types, idx - 1>::type type;
};

/// return type at index idx in typelist TL (specialisation 1st type)
template <typename TL>
struct at<TL, 0> {
    typedef typename TL::head_type type;
};

// test at<typelist<...>, index> implementation
static_assert(std::is_same<int,
	at<typelist<int, float, double>, 0>::type >::value,
	"typelist index 0 does not return correct element");
static_assert(std::is_same<float,
	at<typelist<int, float, double>, 1>::type >::value,
	"typelist index 1 does not return correct element");
static_assert(std::is_same<double,
	at<typelist<int, float, double>, 2>::type >::value,
	"typelist index 2 does not return correct element");

/// imlpementation: find first occurrence of type NEEDLE in typelist
template <std::size_t N, typename TL, typename NEEDLE, typename HEAD>
struct find_impl {
    enum { index = find_impl<N + 1, typename TL::tail_types,
	NEEDLE, typename TL::head_type>::index };
};
/// specialisation (NEEDLE found): find NEEDLE in typelist
template <std::size_t N, typename TL, typename NEEDLE>
struct find_impl<N, TL, NEEDLE, NEEDLE> {
    enum { index = N };
};
/// specialisation (NEEDLE not found): find NEEDLE in typelist
template <std::size_t N, typename TL, typename NEEDLE>
struct find_impl<N, TL, NEEDLE, null_type> {
    enum { index = -1 };
};

/// find index of type NEEDLE in typelist TL (returns -1 if not found)
template <typename TL, typename NEEDLE>
struct find {
    enum { index = find_impl<0, typename TL::tail_types,
	NEEDLE, typename TL::head_type>::index };
};

// test find
static_assert(-1 == find<typelist<>, int>::index,
	"find on empty typelist broken");
static_assert(0 == find<typelist<int>, int>::index,
	"find on one element typelist broken");
static_assert(0 == find<typelist<int, int>, int>::index,
	"find on two element typelist broken");
static_assert(0 == find<typelist<int, float>, int>::index,
	"find on two element typelist broken");
static_assert(1 == find<typelist<float, int>, int>::index,
	"find on two element typelist broken");
static_assert(1 == find<typelist<bool, int, float>, int>::index,
	"find on three element typelist broken");
static_assert(2 == find<typelist<double, bool, int, float>, int>::index,
	"find on four element typelist broken");
static_assert(3 == find<typelist<double, float, bool, int>, int>::index,
	"find on four element typelist broken");
static_assert(-1 == find<typelist<double, float, bool, int>, char>::index,
	"find on four element typelist broken");

/// type to "wrap" other types (to  "distinguish" instances of same type)
template<typename T> struct wrap_type;
/// specialisation: wrapping a wrap_type results in the type itself
template<typename T> struct wrap_type<wrap_type<T> >
{ typedef struct {} wrap_tag; typedef T type; };
/// specialisation: wrap a type
template<typename T> struct wrap_type
{ typedef struct {} wrap_tag; typedef T type; };

// test wrap_type
static_assert(std::is_same<int, typename wrap_type<int>::type>::value,
	"wrap_type is buggy");
static_assert(std::is_same<int,
	typename wrap_type<wrap_type<int> >::type>::value,
	"wrap_type is not idempotent");

// test wrapping and interaction with typelists
namespace {
    typedef struct : public wrap_type<double> {} xAtYEq0;
    typedef struct : public wrap_type<double> {} zAtYEq0;
    typedef struct : public wrap_type<double> {} y;
    typedef struct : public wrap_type<double> {} dxdy;
    typedef struct : public wrap_type<double> {} dzdy;

    typedef typelist<xAtYEq0, zAtYEq0, y, dxdy, dzdy> hitfields;
    static_assert(0 == find<hitfields, xAtYEq0>::index,
	    "lookup with typedefs won't work");
    static_assert(1 == find<hitfields, zAtYEq0>::index,
	    "lookup with typedefs won't work");
    static_assert(2 == find<hitfields, y>::index,
	    "lookup with typedefs won't work");
    static_assert(3 == find<hitfields, dxdy>::index,
	    "lookup with typedefs won't work");
    static_assert(4 == find<hitfields, dzdy>::index,
	    "lookup with typedefs won't work");
    static_assert(std::is_same<double, at<hitfields, 2>::type::type>::value,
	    "unpacking tagged type doesn't work");
}


#include <tuple>

/// base class to convert typelist to tuple (helper templates)
struct typelist_helpers {
    /// little helper needed to unwrap wrapped types (recognises wrapped types)
    template <typename T>
    struct is_wrapped {
	template <typename U> static int* test(typename U::wrap_tag*);
	template <typename U> static int test(...);
	enum { value = std::is_pointer<decltype(test<T>(nullptr))>::value };
    };

    /// little helper to "unwrap" wrapped types (wrap_type, see above)
    template <typename T, bool ISWRAPPED = is_wrapped<T>::value>
    struct unwrap;
    /// specialisation: unwrap a wrapped type
    template <typename T>
    struct unwrap<T, true> { typedef typename T::type type; };
    /// specialisation: unwrap an un-wrapped type (just returns T)
    template <typename T>
    struct unwrap<T, false> { typedef T type; };

    /// little helper to perform the typelist to tuple conversion
    template <typename H, typename... T>
    struct tuple_push;
    /// specialisation
    template <typename H, typename... T>
    struct tuple_push<H, std::tuple<T...> > {
	typedef std::tuple<H, T...> type;
    };
};

/// tuple type holding the types contained in the typelist
template <typename TL>
struct typelist_to_tuple : public typelist_helpers {
    /// finished tuple type
    typedef typename tuple_push<typename unwrap<typename TL::head_type>::type,
	    typename typelist_to_tuple<
		    typename TL::tail_types>::type>::type type;
};

/// finished tuple type for one-element typelists
template <typename H>
struct typelist_to_tuple<typelist<H> > : public typelist_helpers {
    typedef std::tuple<typename unwrap<H>::type> type;
};

namespace {
    // test typelist_to_tuple on typelists with (un) wrapped types
    constexpr static typename typelist_to_tuple<
	typelist<int, int, float> >::type dummy(42, 17, 3.141592f);
    // test typelist_to_tuple on typelists with wrapped types
    constexpr static typename typelist_to_tuple<
	hitfields>::type dummy2(0., 1., 2., 3., 4.);
}

/// tuple type holding references to the types contained in the typelist
template <typename TL>
struct typelist_to_reftuple : public typelist_helpers {
    /// finished tuple type
    typedef typename tuple_push<typename unwrap<typename TL::head_type>::type&,
	    typename typelist_to_reftuple<
		    typename TL::tail_types>::type>::type type;
};

/// finished tuple type for one-element typelists
template <typename H>
struct typelist_to_reftuple<typelist<H> > : public typelist_helpers {
    typedef std::tuple<typename unwrap<H>::type&> type;
};

/// tuple type holding references to the types contained in the typelist
template <typename TL>
struct typelist_to_creftuple : public typelist_helpers {
    /// finished tuple type
    typedef typename tuple_push<const typename unwrap<typename TL::head_type>::type&,
	    typename typelist_to_creftuple<
		    typename TL::tail_types>::type>::type type;
};

/// finished tuple type for one-element typelists
template <typename H>
struct typelist_to_creftuple<typelist<H> > : public typelist_helpers {
    typedef std::tuple<const typename unwrap<H>::type&> type;
};

#include <vector>

/// helper to turn a type T into CONTAINER<T>
template <template <typename...> class CONTAINER = std::vector>
struct containerify {
    template <typename T>
    struct of_type {
	typedef CONTAINER<T> type;
    };
};

/// tuple type holding the containers of the types contained in the typelist
template <typename TL, typename CONTAINERIFIER = containerify<> >
struct typelist_to_tuple_of_containers : public typelist_helpers {
    /// finished tuple type
    typedef typename tuple_push<typename CONTAINERIFIER::template of_type<
	typename unwrap<typename TL::head_type>::type>::type,
	    typename typelist_to_tuple_of_containers<
		    typename TL::tail_types>::type>::type type;
};

/// finished tuple type for one-element typelists
template <typename H, typename CONTAINERIFIER>
struct typelist_to_tuple_of_containers<typelist<H>, CONTAINERIFIER> :
    public typelist_helpers
{
    typedef std::tuple<typename CONTAINERIFIER::template of_type<
	typename unwrap<H>::type>::type> type;
};

namespace {
    static typename typelist_to_tuple_of_containers<
	hitfields>::type dummy3(
		{  0.,  1.,  2.,  3.,  4.,  5.,  6.,  7.,  8.,  9. },
		{ 10., 11., 12., 13., 14., 15., 16., 17., 18., 19. },
		{ 20., 21., 22., 23., 24., 25., 26., 27., 28., 29. },
		{ 30., 31., 32., 33., 34., 35., 36., 37., 38., 39. },
		{ 40., 41., 42., 43., 44., 45., 46., 47., 48., 49. });
}

template <std::size_t N>
struct recursive_apply_tuple
{
    template <typename OBJ, typename F, typename C, typename I>
    auto operator()(OBJ& obj, const F& functor,
	    const C& combiner, I initial) const -> decltype(
		combiner(
		    recursive_apply_tuple<N - 2>()(obj, functor, combiner, initial),
		    functor(std::get<N - 1>(obj))))
    {
	return combiner(
		recursive_apply_tuple<N - 2>()(obj, functor, combiner, initial),
		functor(std::get<N - 1>(obj)));
    }
};

template <>
struct recursive_apply_tuple<1>
{
    template <typename OBJ, typename F, typename C, typename I>
    auto operator()(OBJ& obj, const F& functor,
	    const C& combiner, I initial) const -> decltype(
		combiner(initial, functor(std::get<0>(obj))))
    { return combiner(initial, functor(std::get<0>(obj))); }
};

template <>
struct recursive_apply_tuple<0>
{
    template <typename OBJ, typename F, typename C, typename I>
    void operator()(OBJ&, const F&, const C&, I initial) const
    { return initial; }
};

#include <stdexcept>
#include <array>
#include <algorithm>
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
    public:
	/// shorthand to refer to the type of this class
	typedef SOAIterator<T> self_type;

	/// constructor
	SOAIterator(const T& ptr) noexcept : m_base(ptr) { }
	/// move constructor
	SOAIterator(T&& ptr) noexcept : m_base(std::move(ptr)) { }

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
template <typename T>
SOAIterator<T> operator+(
	typename T::difference_type dist, const SOAIterator<T>& other) noexcept
{ return SOAIterator<T>(other) += dist; }

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
 * points[0].get<point_x> *= 2.0;
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
template <template <typename...> class CONTAINER, typename... FIELDS>
class SOAContainer {
    public:
	static_assert(1 <= sizeof...(FIELDS),
		"need to supply at least on field");

	/// type to represent sizes and indices
	typedef std::size_t size_type;
	/// type to represent differences of indices
	typedef std::ptrdiff_t difference_type;

    private:
	/// type of the storage backend
	typedef typename typelist_to_tuple_of_containers<
	    typelist<FIELDS...>, containerify<CONTAINER> >::type SOAStorage;

	/// storage backend
	SOAStorage m_storage;

#if 1	// C++14 Compile-time integer sequences -- this can go once we use C++14...
	// #include <utility> // defines (in C++14) std::make_index_sequence and std::index_sequence
	template<size_type... indexes> struct index_sequence {
	    static size_type size() { return sizeof...(indexes); }
	};

	template<size_type currentIndex, size_type...indexes> struct make_index_sequence_helper;

	template<size_type...indexes> struct make_index_sequence_helper<0, indexes...> {
	    typedef index_sequence<indexes...> type;
	};

	template<size_type currentIndex, size_type...indexes> struct make_index_sequence_helper {
	    typedef typename make_index_sequence_helper<currentIndex - 1, currentIndex - 1, indexes...>::type type;
	};

	template<size_type N> struct make_index_sequence : public make_index_sequence_helper<N>::type { };
#endif
	/// little helper for indexing to implement clear()
	struct clearHelper {
	    template <typename T>
	    bool operator()(T& obj) const { obj.clear(); return true; }
	};

	/// little helper for indexing to implement pop_back()
	struct pop_backHelper {
	    template <typename T>
	    bool operator()(T& obj) const { obj.pop_back(); return true; }
	};

	/// little helper for indexing to implement shrink_to_fit()
	struct shrink_to_fitHelper {
	    template <typename T>
	    bool operator()(T& obj) const { obj.shrink_to_fit(); return true; }
	};

	/// little helper for indexing to implement reserve()
	struct reserveHelper {
	    size_type m_sz;
	    reserveHelper(size_type sz) : m_sz(sz) { }
	    template <typename T>
	    bool operator()(T& obj) const { obj.reserve(m_sz); return true; }
	};

	/// little helper for indexing to implement capacity()
	struct capacityHelper {
	    template <typename T>
	    size_type operator()(T& obj) const { return obj.capacity(); }
	};

	/// little helper for indexing to implement max_size()
	struct max_sizeHelper {
	    template <typename T>
	    size_type operator()(T& obj) const { return obj.max_size(); }
	};

    public:
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
	class SOAObjectProxy {
	    public:
		typedef typename typelist_to_tuple<typelist<FIELDS...> >::type value_type;
		typedef difference_type difference_type;
	    private:
		SOAStorage* m_storage;	///< underlying SOA storage of members
		size_type m_index;	///< index into underlying SOA storage

		// SOAContainer is allowed to invoke the private constructor
		friend class SOAContainer<CONTAINER, FIELDS...>;
		/// constructor is private, but parent container is a friend
		SOAObjectProxy(
			SOAStorage* storage = nullptr,
			size_type index = 0) noexcept :
		    m_storage(storage), m_index(index)
	        { }

		/// typedef for tuple of references to members
		typedef typename typelist_to_reftuple<typelist<FIELDS...> >::type reference_type;

		/// little helper to implement conversion to tuple
		template <size_type... Indexes>
		value_type to_value() const
		{ return std::tie(std::get<Indexes...>(m_storage)[m_index]); }
		/// little helper to implement operator=
		template <size_type... Indexes>
		reference_type to_reference()
		{ return std::tie(std::get<Indexes...>(m_storage)[m_index]); }

	    public:
		/// access to member by number
		template <size_type MEMBERNO>
		auto get() -> decltype(std::get<MEMBERNO>(*m_storage)[m_index])
		{ return std::get<MEMBERNO>(*m_storage)[m_index]; }
		/// access to member by "member tag"
		template <typename MEMBER>
		auto get() -> decltype(std::get<find<
			typelist<FIELDS...>, MEMBER>::index>(*m_storage)[m_index])
		{
		    return std::get<
			find<typelist<FIELDS...>, MEMBER>::index>(
				*m_storage)[m_index];
		}
		/// access to member by number (read-only)
		template <size_type MEMBERNO>
		auto get() const ->
		    decltype(std::get<MEMBERNO>(*m_storage)[m_index])
		{ return std::get<MEMBERNO>(*m_storage)[m_index]; }
		/// access to member by "member tag" (read-only)
		template <typename MEMBER>
		auto get() const -> decltype(std::get<find<
			typelist<FIELDS...>, MEMBER>::index>(*m_storage)[m_index])
		{
		    return std::get<
			find<typelist<FIELDS...>, MEMBER>::index>(
				*m_storage)[m_index];
		}

		/// convert to tuple of member contents
		operator value_type() const
		{ return to_value<make_index_sequence<sizeof...(FIELDS)> >(); }
		/// assign from tuple of member contents
		SOAObjectProxy& operator=(const value_type& other)
		{ to_reference<sizeof...(FIELDS)>() = other; return *this; }

		/// "deep"-swap the contents of two SOAObjectProxy instances
		void swap(SOAObjectProxy& other)
		{ std::swap(to_reference<sizeof...(FIELDS)>(), other.to_reference<sizeof...(FIELDS)>()); }

		/// dereference
		SOAObjectProxy& operator*() noexcept
		{ return *this; }
		/// dereference
		const SOAObjectProxy& operator*() const noexcept
		{ return *this; }
		/// helper to support iterator->get<...>() idiom
		SOAObjectProxy* operator->() noexcept
		{ return this; }
		/// helper to support iterator->get<...>() idiom
		const SOAObjectProxy* operator->() const noexcept
		{ return this; }

		/// move to next element (pre-increment)
		SOAObjectProxy& operator++() noexcept
		{ ++m_index; return *this; }
		/// move to previous element (pre-decrement)
		SOAObjectProxy& operator--() noexcept
		{ --m_index; return *this; }
		/// move to next element (post-increment)
		SOAObjectProxy operator++(int) noexcept
		{ SOAObjectProxy retVal(*this); operator++(); return retVal; }
		/// move to previous element (post-decrement)
		SOAObjectProxy operator--(int) noexcept
		{ SOAObjectProxy retVal(*this); operator--(); return retVal; }
		
		/// advance dist elements
		SOAObjectProxy& operator+=(difference_type dist) noexcept
		{ m_index += dist; return *this; }
		/// retreat dist elements
		SOAObjectProxy& operator-=(difference_type dist) noexcept
		{ m_index -= dist; return *this; }
		/// advance dist elements
		SOAObjectProxy operator+(difference_type dist) const noexcept
		{ return SOAObjectProxy(*this) += dist; }
		/// retreat dist elements
		SOAObjectProxy operator-(difference_type dist) const noexcept
		{ return SOAObjectProxy(*this) -= dist; }
		/// distance between two SOAObjectProxy instances
		difference_type operator-(const SOAObjectProxy& other) const noexcept
		{
		    return m_storage == other.m_storage ?
			(m_index - other.m_index) : -1;
		}

		/// check if SOAObjectProxy is valid (i.e. points to something)
		operator bool() const noexcept
		{
		    return nullptr != m_storage &&
			m_index < std::get<0>(*m_storage).size();
		}
		/// check for equality
		bool operator==(const SOAObjectProxy& other) const noexcept
		{
		    return m_storage == other.m_storage &&
			m_index == other.m_index;
		}
		/// check for inequality
		bool operator!=(const SOAObjectProxy& other) const noexcept
		{
		    return m_storage != other.m_storage ||
			m_index != other.m_index;
		}
		/// ordering comparison: <
		bool operator<(const SOAObjectProxy& other) const noexcept
		{
		    return (m_storage < other.m_storage) ? true :
			(other.m_storage < m_storage) ? false :
			m_index < other.m_index;
		}
		/// ordering comparison: <=
		bool operator<=(const SOAObjectProxy& other) const noexcept
		{
		    return (m_storage < other.m_storage) ? true :
			(other.m_storage < m_storage) ? false :
			m_index <= other.m_index;
		}
		/// ordering comparison: >
		bool operator>(const SOAObjectProxy& other) const noexcept
		{
		    return (m_storage < other.m_storage) ? false :
			(other.m_storage < m_storage) ? true :
			other.m_index < m_index;
		}
		/// ordering comparison: >=
		bool operator>=(const SOAObjectProxy& other) const noexcept
		{
		    return (m_storage < other.m_storage) ? false :
			(other.m_storage < m_storage) ? true :
			other.m_index <= m_index;
		}
	}; 

	/// (notion of) type of the contained objects
	typedef typename SOAObjectProxy::value_type value_type;
	/// reference to contained objects
	typedef SOAObjectProxy reference_type;
	/// pointer to contained objects
	typedef SOAObjectProxy pointer_type;
	/// reference to contained objects
	typedef const SOAObjectProxy const_reference_type;
	/// const pointer to contained objects
	typedef const SOAObjectProxy const_pointer_type;
	/// iterator type
	typedef SOAIterator<SOAObjectProxy> iterator;
	/// const iterator type
	typedef SOAIterator<const SOAObjectProxy> const_iterator;
	/// reverse iterator type
	typedef std::reverse_iterator<iterator> reverse_iterator;
	/// const reverse iterator type
	typedef std::reverse_iterator<const_iterator> const_reverse_iterator;

	/// return if the container is empty
	bool empty() const { return std::get<0>(m_storage).empty(); }
	/// return the size of the container
	size_type size() const { return std::get<0>(m_storage).size(); }

	/// clear the container
	void clear()
	{
	    recursive_apply_tuple<sizeof...(FIELDS)>()(m_storage,
		    clearHelper(), [] (bool, bool) { return true; }, true);
       	}

	/// pop the last element off the container
	void pop_back()
	{
	    recursive_apply_tuple<sizeof...(FIELDS)>()(m_storage,
		    pop_backHelper(), [] (bool, bool) { return true; }, true);
       	}

	/// shrink the underlying storage of the container to fit its size
	void shrink_to_fit()
	{
	    recursive_apply_tuple<sizeof...(FIELDS)>()(m_storage,
		    shrink_to_fitHelper(), [] (bool, bool) { return true; }, true);
       	}

	/// reserve space for at least sz elements
	void reserve(size_type sz)
	{
	    recursive_apply_tuple<sizeof...(FIELDS)>()(m_storage,
		    reserveHelper(sz), [] (bool, bool) { return true; }, true);
       	}

	/// return capacity of container
	size_type capacity() const
	{
	    return recursive_apply_tuple<sizeof...(FIELDS)>()(m_storage,
		    capacityHelper(), [] (size_type s1, size_type s2) {
		    return std::min(s1, s2); }, size_type(-1));
       	}

	/// return maximal size of container
	size_type max_size() const
	{
	    return recursive_apply_tuple<sizeof...(FIELDS)>()(m_storage,
		    max_sizeHelper(), [] (size_type s1, size_type s2) {
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
	    throw std::out_of_range("out of bounds");
	}
	/// access specified element with out of bounds checking (read-only)
	const_reference_type at(size_type idx) const
	{
	    if (idx < size()) return operator[](idx);
	    throw std::out_of_range("out of bounds");
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

	iterator insert(const_iterator pos, const value_type& val)
	{
	    insert_helper(pos.m_index, val);
	    return iterator(*pos + 1);
	}
};

#include <cassert>
static void test()
{
    SOAContainer<std::vector, double, int, int> c;
    const SOAContainer<std::vector, double, int, int>& cc = c;
    // check basic properties
    assert(c.empty());
    assert(0 == c.size());
    c.clear();
    assert(1 <= c.max_size());
    assert(0 <= c.capacity());
    // reserve space
    c.reserve(64);
    assert(64 <= c.capacity());
    assert(c.capacity() <= c.max_size());
    // check iterators
    assert(!c.begin());
    assert(c.begin() == c.end());
    assert(cc.begin() == cc.end());
    assert(c.begin() == cc.begin());
    assert(c.begin() <= c.end());
    assert(cc.begin() <= cc.end());
    assert(c.begin() <= cc.begin());
    assert(c.begin() >= c.end());
    assert(cc.begin() >= cc.end());
    // check reverse iterators
    assert(c.rbegin() >= cc.rbegin());
    assert(c.rbegin() == c.rend());
    assert(cc.rbegin() == cc.rend());
    assert(c.rbegin() == cc.rbegin());
    assert(c.rbegin() <= c.rend());
    assert(cc.rbegin() <= cc.rend());
    assert(c.rbegin() <= cc.rbegin());
    assert(c.rbegin() >= c.rend());
    assert(cc.rbegin() >= cc.rend());
    assert(c.rbegin() >= cc.rbegin());
}
