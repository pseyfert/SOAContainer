/** @file SOAUtils.h
 *
 * @author Manuel Schiller <Manuel.Schiller@cern.ch>
 * @date 2015-04-09
 */

#ifndef SOAUTILS_H
#define SOAUTILS_H

#include <cstdint>

#include <tuple>

/// various other utilities used by SOAContainer
namespace SOAUtils {
    /// apply some functor to each element of a tuple, and gather return value
    template <std::size_t N>
    struct recursive_apply_tuple
    {
	struct IndexWrapper { enum { value = N - 1 }; };
        template <typename OBJ, typename F, typename C, typename I>
        auto operator()(OBJ& obj, const F& functor,
		const C& combiner, I initial) const -> decltype(
		    combiner(
			recursive_apply_tuple<N - 1>()(obj, functor,
			    combiner, initial),
			functor(std::get<N - 1>(obj), IndexWrapper())))
        {
	    return combiner(
    		recursive_apply_tuple<N - 1>()(obj, functor,
		    combiner, initial),
    		functor(std::get<N - 1>(obj), IndexWrapper()));
        }
    };
    
    template <>
    struct recursive_apply_tuple<0>
    {
        template <typename OBJ, typename F, typename C, typename I>
        I operator()(OBJ&, const F&, const C&, I initial) const
        { return initial; }
    };

#if 1
    // C++14 Compile-time integer sequences -- this can go once we use C++14...
    // #include <utility> // defines (in C++14) std::make_index_sequence and std::index_sequence
	
    /// C++14-like index sequence
    template<std::size_t... indexes> struct index_sequence {
	static std::size_t size() { return sizeof...(indexes); }
    };

    template<std::size_t currentIndex, std::size_t...indexes> struct make_index_sequence_helper;

    template<std::size_t...indexes> struct make_index_sequence_helper<0, indexes...> {
	typedef index_sequence<indexes...> type;
    };

    template<std::size_t currentIndex, std::size_t...indexes> struct make_index_sequence_helper {
	typedef typename make_index_sequence_helper<currentIndex - 1, currentIndex - 1, indexes...>::type type;
    };

    template<std::size_t N> struct make_index_sequence : public make_index_sequence_helper<N>::type { };
#endif
}

#endif // SOAUTILS_H

// vim: sw=4:tw=78:ft=cpp
