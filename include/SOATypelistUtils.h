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

namespace SOATypelist {
    /// find index of type NEEDLE in typelist TL (returns -1 if not found)
    template <typename TL, typename NEEDLE>
    struct find { enum { index = TL::template find<NEEDLE>() }; };

    // compile-time test find
    static_assert(std::size_t(-1) == find<typelist<>, int>::index,
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
    static_assert(std::size_t(-1) == find<typelist<double, float, bool, int>, char>::index,
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

    // compile-time test wrapping and interaction with typelists
    namespace __impl_compile_time_tests {
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

    // compile-time test typelist_to_tuple
    namespace __impl_compile_time_tests {
	// test typelist_to_tuple on typelists with unwrapped types
	static_assert(std::is_same<std::tuple<int, int, float>,
		typename typelist_to_tuple<
		typelist<int, int, float> >::type>::value,
		"typelist_to_tuple broken for simple typelists");
	// test typelist_to_tuple on typelists with wrapped types
	static_assert(std::is_same<
		std::tuple<double, double, double, double, double>,
		typename typelist_to_tuple<hitfields>::type>::value,
		"typelist_to_tuple broken for wrapped typelists");
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

    // compile-time test typelist_to_reftuple
    namespace __impl_compile_time_tests {
	// test typelist_to_tuple on typelists with unwrapped types
	static_assert(std::is_same<std::tuple<int&, int&, float&>,
		typename typelist_to_reftuple<
		typelist<int, int, float> >::type>::value,
		"typelist_to_reftuple broken for simple typelists");
	// test typelist_to_tuple on typelists with wrapped types
	static_assert(std::is_same<
		std::tuple<double&, double&, double&, double&, double&>,
		typename typelist_to_reftuple<hitfields>::type>::value,
		"typelist_to_reftuple broken for wrapped typelists");
    }

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

    // compile-time test typelist_to_creftuple
    namespace __impl_compile_time_tests {
	// test typelist_to_tuple on typelists with unwrapped types
	static_assert(std::is_same<std::tuple<
		const int&, const int&, const float&>,
		typename typelist_to_creftuple<
		typelist<int, int, float> >::type>::value,
		"typelist_to_creftuple broken for simple typelists");
	// test typelist_to_tuple on typelists with wrapped types
	static_assert(std::is_same<std::tuple<const double&, const double&,
		const double&, const double&, const double&>,
		typename typelist_to_creftuple<hitfields>::type>::value,
		"typelist_to_creftuple broken for wrapped typelists");
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
