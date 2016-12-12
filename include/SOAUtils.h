/** @file SOAUtils.h
 *
 * @author Manuel Schiller <Manuel.Schiller@cern.ch>
 * @date 2015-04-09
 */

#ifndef SOAUTILS_H
#define SOAUTILS_H

#include <tuple>
#include <utility>
#include <cstdint>
#include <type_traits>

#include "c++14_compat.h"

/// various other utilities used by SOAContainer
namespace SOAUtils {
    /// invoke fun on given arguments, return a dummy int if fun returns void
    template <typename FUN, typename... ARG>
    auto invoke_void2int(FUN fun, ARG&&... arg) noexcept(
        noexcept(fun(std::forward<ARG>(arg)...))) ->
        typename std::enable_if<
            !std::is_same<void, typename std::result_of<FUN(ARG...)>::type>::value,
            typename std::result_of<FUN(ARG...)>::type>::type
    { return fun(std::forward<ARG>(arg)...); }
    /// invoke fun on given arguments, return a dummy int if fun returns void
    template <typename FUN, typename... ARG>
    auto invoke_void2int(FUN fun, ARG&&... arg) noexcept(
        noexcept(fun(std::forward<ARG>(arg)...))) ->
        typename std::enable_if<
            std::is_same<void, typename std::result_of<FUN(ARG...)>::type>::value,
            int>::type
    { fun(std::forward<ARG>(arg)...); return 0; }

    /// apply functor fn to each element of tuple, and return tuple with results
    template <typename FUNCTOR, typename OBJ, std::size_t... IDX>
    auto map(FUNCTOR fn, OBJ&& obj, std::index_sequence<IDX...>) noexcept(
        noexcept(std::make_tuple(invoke_void2int(fn, std::get<IDX>(std::forward<OBJ>(obj)))...))) ->
        decltype(std::make_tuple(invoke_void2int(fn, std::get<IDX>(std::forward<OBJ>(obj)))...))
    { return std::make_tuple(invoke_void2int(fn, std::get<IDX>(std::forward<OBJ>(obj)))...); }

    /// apply functor fn to each element of tuple, and return tuple with results
    template <typename FUNCTOR, typename OBJ>
    auto map(FUNCTOR fn, OBJ&& obj) noexcept(
        noexcept(map(fn, std::forward<OBJ>(obj),
                std::make_index_sequence<
                std::tuple_size<typename std::decay<OBJ>::type>::value>()))) ->
        decltype(map(fn, std::forward<OBJ>(obj),
                std::make_index_sequence<
                std::tuple_size<typename std::decay<OBJ>::type>::value>()))
    {
        return map(fn, std::forward<OBJ>(obj),
            std::make_index_sequence<
            std::tuple_size<typename std::decay<OBJ>::type>::value>());
    }

    /// apply functor fn to each element of tuple, and return tuple with results
    template <typename OBJ, typename FUNCTOR, typename ARG2, std::size_t... IDX>
    auto apply_tuple2(OBJ&& obj, FUNCTOR fn, ARG2&& arg2, std::index_sequence<IDX...>) noexcept(
        noexcept(std::make_tuple(invoke_void2int(fn, std::get<IDX>(std::forward<OBJ>(obj)), std::get<IDX>(std::forward<ARG2>(arg2)))...))) ->
        decltype(std::make_tuple(invoke_void2int(fn, std::get<IDX>(std::forward<OBJ>(obj)), std::get<IDX>(std::forward<ARG2>(arg2)))...))
    { return std::make_tuple(invoke_void2int(fn, std::get<IDX>(std::forward<OBJ>(obj)), std::get<IDX>(std::forward<ARG2>(arg2)))...); }
    /// apply functor fn to each element of tuple, and return tuple with results
    template <typename OBJ, typename FUNCTOR, typename ARG2, typename ARG3, std::size_t... IDX>
    auto apply_tuple3(OBJ&& obj, FUNCTOR fn, ARG2&& arg2, ARG3&& arg3, std::index_sequence<IDX...>) noexcept(
        noexcept(std::make_tuple(invoke_void2int(fn, std::get<IDX>(std::forward<OBJ>(obj)), std::get<IDX>(std::forward<ARG2>(arg2)), std::forward<ARG3>(arg3))...))) ->
        decltype(std::make_tuple(invoke_void2int(fn, std::get<IDX>(std::forward<OBJ>(obj)), std::get<IDX>(std::forward<ARG2>(arg2)), std::forward<ARG3>(arg3))...))
    { return std::make_tuple(invoke_void2int(fn, std::get<IDX>(std::forward<OBJ>(obj)), std::get<IDX>(std::forward<ARG2>(arg2)), std::forward<ARG3>(arg3))...); }

    /// implementation details of foldl
    namespace foldl_impl {
        /// implementation of foldl
        template <typename FUN, typename TUP, std::size_t LEN>
        struct foldl_impl {
            FUN fun;
            const TUP& tup;
            /// fold over tuple with more than 2 elements
            template <typename INI, std::size_t IDX, std::size_t... IDXS>
            typename std::enable_if<(LEN > 1),
                typename std::result_of<
                    foldl_impl<FUN, TUP, LEN - 1>(
                        typename std::result_of<FUN(INI,
                            typename std::tuple_element<IDX, TUP>::type)
                        >::type, std::index_sequence<IDXS...>)
                    >::type
                >::type
            operator()(INI ini,
                    std::index_sequence<IDX, IDXS...>) const noexcept(
                    noexcept(foldl_impl<FUN, TUP, LEN - 1>{fun, tup}(
                            fun(ini, std::get<IDX>(tup)),
                            std::index_sequence<IDXS...>())))
            {
                return foldl_impl<FUN, TUP, LEN - 1>{fun, tup}(
                        fun(ini, std::get<IDX>(tup)),
                        std::index_sequence<IDXS...>());
            }
            /// foldl over tuple with exactly one element
            template <typename INI, std::size_t IDX>
            typename std::enable_if<1 == LEN,
                typename std::result_of<
                    FUN(INI,
                            typename std::tuple_element<IDX, TUP>::type)>::type
                >::type
            operator()(INI ini, std::index_sequence<IDX>) const noexcept(
                    noexcept(fun(ini, std::get<IDX>(tup))))
            { return fun(ini, std::get<IDX>(tup)); }
            /// foldl over tuple with no elements
            template <typename INI>
            INI operator()(INI ini, std::index_sequence<>) const noexcept(
                    noexcept(INI(ini)))
            { return ini; }
        };
    }
    
    /** @brief foldl (left fold) on tuples
     *
     * Given an index sequence i1, ..., iN, a tuple with elements (e_1, ...,
     * e_N-1, e_N), a fold function fun, and an initial value ini, "fold" the
     * tuple elements given in the index sequence using the functor fun
     * starting from the left, i.e. return fun(fun(fun(...fun(ini, e_i1),
     * ...), e_iN-1), e_iN), or return ini for the empty tuple.
     *
     * @tparam INI  type of initial value
     * @tparam FUN  type of functor used in the fold
     * @tparam TUP  type of the tuple to be folded
     *
     * @param fun   functor used to "fold" tuple
     * @param tup   tuple to fold
     * @param ini   initial value
     *
     * @returns     value of the left fold
     *
     * @author Manuel Schiller <Manuel.Schiller@glasgow.ac.uk>
     * @date 2016-12-12
     */
    template <typename INI, typename FUN, typename TUP, std::size_t... IDXS>
    typename std::result_of<foldl_impl::foldl_impl<FUN, TUP, sizeof...(IDXS)>(INI, std::index_sequence<IDXS...>)>::type
    foldl(FUN fun, const TUP& tup, INI ini, std::index_sequence<IDXS...>) noexcept(
            noexcept(foldl_impl::foldl_impl<FUN, TUP, sizeof...(IDXS)>{fun, tup}(
                    ini, std::index_sequence<IDXS...>())))
    {
        return foldl_impl::foldl_impl<FUN, TUP, sizeof...(IDXS)>{fun, tup}(
                ini, std::index_sequence<IDXS...>());
    }
    
    /** @brief foldl (left fold) on tuples
     *
     * Given a tuple with elements (e_1, ..., e_N-1, e_N), a fold function
     * fun, and an initial value ini, "fold" the tuple using the functor fun
     * starting from the left, i.e. return fun(fun(fun(...fun(ini, e_1), ...),
     * e_N-1), e_N), or return ini for the empty tuple.
     *
     * @tparam INI  type of initial value
     * @tparam FUN  type of functor used in the fold
     * @tparam TUP  type of the tuple to be folded
     *
     * @param fun   functor used to "fold" tuple
     * @param tup   tuple to fold
     * @param ini   initial value
     *
     * @returns     value of the left fold
     *
     * @author Manuel Schiller <Manuel.Schiller@glasgow.ac.uk>
     * @date 2016-12-12
     */
    template <typename INI, typename FUN, typename TUP>
    auto foldl(FUN fun, const TUP& tup, INI ini = INI()) noexcept(
            noexcept(foldl(fun, tup, ini,
                    std::make_index_sequence<std::tuple_size<TUP>::value>()))) ->
        decltype(foldl(fun, tup, ini,
                    std::make_index_sequence<std::tuple_size<TUP>::value>()))
    {
        return foldl(fun, tup, ini,
                std::make_index_sequence<std::tuple_size<TUP>::value>());
    }

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
}

#endif // SOAUTILS_H

// vim: sw=4:tw=78:ft=cpp:et
