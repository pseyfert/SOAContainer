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
	/// used to pass current index to functor
	struct IndexWrapper { enum { value = N - 1 }; };
	/// version for functors that return something
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
	/// version for functors that return nothing
        template <typename OBJ, typename F>
	void operator()(OBJ& obj, const F& functor) const
        {
	    recursive_apply_tuple<N - 1>()(obj, functor);
	    functor(std::get<N - 1>(obj), IndexWrapper());
        }
    };
    
    /// specialisation for termination of recursion
    template <>
    struct recursive_apply_tuple<0>
    {
	/// version for functors that return something
        template <typename OBJ, typename F, typename C, typename I>
        I operator()(OBJ&, const F&, const C&, I initial) const
        { return initial; }
	/// version for functors that return nothing
        template <typename OBJ, typename F>
        void operator()(OBJ&, const F&) const
	{ }
    };

    /// little tool to call a callable using the arguments given in a tuple
    template <typename F, typename T, size_t sz = std::tuple_size<T>::value>
    struct caller {
        const F& m_fn; ///< callable
        /// constructor
        caller(const F& fn) : m_fn(fn) { }

        /// call operator (unpacks tuple one by one, perfect forwarding)
        template <typename... ARGS>
        auto operator()(T&& tuple, ARGS&&... args) -> decltype(
            caller<F, T, sz - 1>(m_fn)(std::forward<T>(tuple),
                std::move(std::get<sz - 1>(tuple)), std::forward<ARGS>(args)...))
        {
            return caller<F, T, sz - 1>(m_fn)(std::forward<T>(tuple),
                std::move(std::get<sz - 1>(tuple)), std::forward<ARGS>(args)...);
        }

        /// call operator (unpacks tuple one by one)
        template <typename... ARGS>
        auto operator()(const T& tuple, const ARGS&... args) -> decltype(
            caller<F, T, sz - 1>(m_fn)(tuple,
                std::get<sz - 1>(tuple), args...))
        {
            return caller<F, T, sz - 1>(m_fn)(tuple,
                std::get<sz - 1>(tuple), args...);
        }
    };

    /// little tool to call using unpacked tuple arguments (specialisation)
    template <typename F, typename T>
    struct caller<F, T, size_t(0)> {
        const F& m_fn; ///< callable
        /// constructor
        caller(const F& fn) : m_fn(fn) { }

        /// call operator (unpacks tuple one by one, perfect forwarding)
        template <typename... ARGS>
        auto operator()(T&& /* tuple */, ARGS&&... args) -> decltype(
            m_fn(std::forward<ARGS>(args)...))
        { return m_fn(std::forward<ARGS>(args)...); }

        /// call operator (unpacks tuple one by one)
        template <typename... ARGS>
        auto operator()(const T& /* tuple */,
            const ARGS&... args) -> decltype(m_fn(args...))
        { return m_fn(args...); }
    };

    /// little helper to call callable f with contents of t as arguments (perfect forwarding)
    template<typename F, typename T>
    auto call(const F& f, T&& t) -> decltype(
        caller<F, typename std::decay<T>::type>(f)(std::forward<T>(t)))
    { return caller<F, typename std::decay<T>::type>(f)(std::forward<T>(t)); }

    /// little helper to call callable f with contents of t as arguments
    template<typename F, typename T>
    auto call(const F& f, const T& t) -> decltype(
        caller<F, typename std::decay<T>::type>(f)(t))
    { return caller<F, typename std::decay<T>::type>(f)(t); }

#if 0
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
