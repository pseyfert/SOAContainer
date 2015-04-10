/** @file SOATypelist.h
 *
 * @author Manuel Schiller <Manuel.Schiller@cern.ch>
 * @date 2015-10-02
 */

#ifndef SOATYPELIST_H
#define SOATYPELIST_H

#include <cstdint>
#include <type_traits>

namespace SOATypelist {
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

    // compile-time test typelist instantiation and size operations
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

    // compile-time test at<typelist<...>, index> implementation
    static_assert(std::is_same<int,
	    at<typelist<int, float, double>, 0>::type >::value,
	    "typelist index 0 does not return correct element");
    static_assert(std::is_same<float,
	    at<typelist<int, float, double>, 1>::type >::value,
	    "typelist index 1 does not return correct element");
    static_assert(std::is_same<double,
	    at<typelist<int, float, double>, 2>::type >::value,
	    "typelist index 2 does not return correct element");
}

#endif // SOATYPELIST_H

// vim: sw=4:tw=78:ft=cpp
