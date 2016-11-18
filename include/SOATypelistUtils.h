/** @file SOATypelistUtils.h
 *
 * @author Manuel Schiller <Manuel.Schiller@cern.ch>
 * @date 2015-04-10
 */

#ifndef SOATYPELISTUTILS_H
#define SOATYPELISTUTILS_H

#include <tuple>
#include <vector>

#include "SOATypelist.h"
#include "c++14_compat.h"

namespace SOATypelist {
    /// is a type a wrapped type or not (see below)?
    template <typename T, typename = void>
    struct is_wrapped : std::false_type {};
    /// is a type a wrapped type or not (see below)?
    template <typename T>
    struct is_wrapped<T, std::void_t<typename T::wrap_tag> > : std::true_type {};

    /// type to "wrap" other types (to  "distinguish" instances of same type)
    template<typename T, bool DUMMY = is_wrapped<T>::value> struct wrap_type;
    /// specialisation: wrapping a wrap_type results in the type itself
    template<typename T> struct wrap_type<T, true>
    { typedef struct {} wrap_tag; typedef typename T::type type; };
    /// specialisation: wrap a type
    template<typename T> struct wrap_type<T, false>
    { typedef struct {} wrap_tag; typedef T type; };
    /// little helper to "unwrap" wrapped types (wrap_type, see above)
    template <typename T> using unwrap_t = typename wrap_type<T>::type;

    // test wrap_type
    static_assert(std::is_same<int, typename wrap_type<int>::type>::value,
	    "wrap_type is buggy");
    static_assert(std::is_same<int,
	    typename wrap_type<wrap_type<int> >::type>::value,
	    "wrap_type is not idempotent");

    // compile-time test wrapping and interaction with typelists
    namespace __impl_compile_time_tests {
	typedef struct : public wrap_type<double> {} xAtYEq0;
	typedef struct : public wrap_type<double> {} zAtYEq0;
	typedef struct : public wrap_type<double> {} y;
	typedef struct : public wrap_type<double> {} dxdy;
	typedef struct : public wrap_type<double> {} dzdy;

	typedef typelist<xAtYEq0, zAtYEq0, y, dxdy, dzdy> hitfields;
	static_assert(0 == hitfields::find<xAtYEq0>(),
		"lookup with typedefs won't work");
	static_assert(1 == hitfields::find<zAtYEq0>(),
		"lookup with typedefs won't work");
	static_assert(2 == hitfields::find<y>(),
		"lookup with typedefs won't work");
	static_assert(3 == hitfields::find<dxdy>(),
		"lookup with typedefs won't work");
	static_assert(4 == hitfields::find<dzdy>(),
		"lookup with typedefs won't work");
	static_assert(std::is_same<double, hitfields::at<2>::type::type>::value,
		"unpacking tagged type doesn't work");
	static_assert(std::is_same<typelist<double, double, double, double, double>,
		hitfields::map_t<unwrap_t> >::value,
		"unpacking tagged type doesn't work");

    }

    /// base class to convert typelist to tuple (helper templates)
    struct typelist_helpers {

	/// little helper to perform the typelist to tuple conversion
	template <typename H, typename... T>
	struct tuple_push;
	/// specialisation
	template <typename H, typename... T>
	struct tuple_push<H, std::tuple<T...> > {
	    typedef std::tuple<H, T...> type;
	};
    };

    template <typename TL>
    class to_tuple {
	private:
	    template <typename T> using decay_t = typename std::decay<T>::type;
	    template <typename... ARGS>
	    static std::tuple<ARGS...> to_tuple_fn(typelist<ARGS...>) noexcept;
	    template <typename... ARGS>
	    static std::tuple<ARGS&...> to_ref_tuple_fn(typelist<ARGS...>) noexcept;
	    template <typename... ARGS>
	    static std::tuple<const ARGS&...> to_cref_tuple_fn(typelist<ARGS...>) noexcept;
	    template <typename... ARGS>
	    static std::tuple<ARGS&&...> to_rval_tuple_fn(typelist<ARGS...>) noexcept;
	public:
	    using value_tuple = decltype(to_tuple_fn(typename TL::template map_t<unwrap_t>::template map_t<decay_t>()));
	    using rvalue_tuple = decltype(to_rval_tuple_fn(typename TL::template map_t<unwrap_t>::template map_t<decay_t>()));
	    using reference_tuple = decltype(to_ref_tuple_fn(typename TL::template map_t<unwrap_t>::template map_t<decay_t>()));
	    using const_reference_tuple = decltype(to_cref_tuple_fn(typename TL::template map_t<unwrap_t>::template map_t<decay_t>()));

    };
    /// test implementation of to_tuple
    namespace __impl_compile_time_tests {
	static_assert(std::is_same<
		typename to_tuple<typelist<int, float> >::value_tuple,
		std::tuple<int, float> >::value, "implementation error");
	static_assert(std::is_same<
		typename to_tuple<typelist<int, float> >::reference_tuple,
		std::tuple<int&, float&> >::value, "implementation error");
	static_assert(std::is_same<
		typename to_tuple<typelist<int, float> >::const_reference_tuple,
		std::tuple<const int&, const float&> >::value,
		"implementation error");
	static_assert(std::is_same<
		typename to_tuple<typelist<int, float> >::rvalue_tuple,
		std::tuple<int&&, float&&> >::value, "implementation error");
    }

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
	    unwrap_t<typename TL::head_type> >::type,
		     typename typelist_to_tuple_of_containers<
			 typename TL::tail_types>::type>::type type;
    };

    /// finished tuple type for one-element typelists
    template <typename H, typename CONTAINERIFIER>
    struct typelist_to_tuple_of_containers<typelist<H>, CONTAINERIFIER> :
        public typelist_helpers
    {
	typedef std::tuple<typename CONTAINERIFIER::template of_type<
	    unwrap_t<H> >::type> type;
    };

    // compile-time test typelist_to_tuple_of_containers
    namespace __impl_compile_time_tests {
	// test typelist_to_tuple on typelists with unwrapped types
	static_assert(std::is_same<std::tuple<
		std::vector<int>, std::vector<int>, std::vector<float> >,
		typename typelist_to_tuple_of_containers<
		typelist<int, int, float> >::type>::value,
		"typelist_to_tuple_of_containers broken for simple typelists");
	// test typelist_to_tuple on typelists with wrapped types
	static_assert(std::is_same<std::tuple<std::vector<double>,
		std::vector<double>, std::vector<double>, std::vector<double>,
		std::vector<double> >, typename
		typelist_to_tuple_of_containers<hitfields>::type>::value,
		"typelist_to_tuple_of_containers broken for wrapped typelists");
    }
}

#endif // SOATYPELISTUTILS_H

// vim: sw=4:tw=78:ft=cpp
