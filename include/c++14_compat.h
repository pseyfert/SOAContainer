/** @file c++14_compat.h
 *
 * @brief some required constructs from C++ 14 implemented using C++ 11
 *
 * @author Manuel Schiller <Manuel.Schiller@cern.ch>
 * @date 2016-11-16
 *
 * For copyright and license information, see the end of the file.
 */

#ifndef CPP14_COMPAT_H
#define CPP14_COMPAT_H

#if __cplusplus > 201103L // well, g++ and icpc have strange values here in c++14 mode
// C++14 - use the standard's facilities
#include <utility>
#elif __cplusplus >= 201103L
// C++11 - implement what's needed
// C++14 Compile-time integer sequences -- this can go once we use C++14...
#include <cstddef>
#include <type_traits>
namespace std {
    template<size_t... indexes>
    struct index_sequence {
        static constexpr size_t size() { return sizeof...(indexes); }
    };

    template <size_t... idxs1, size_t... idxs2>
    constexpr static inline
    index_sequence<idxs1..., (sizeof...(idxs1) + idxs2)...>
    __cat_idx_seq(index_sequence<idxs1...>, index_sequence<idxs2...>) noexcept
    { return {}; }

    template <size_t N>
    struct __mk_idx_seq { using type = decltype(__cat_idx_seq(
                std::declval<typename __mk_idx_seq<N - N / 2>::type>(),
                std::declval<typename __mk_idx_seq<N / 2>::type>())); };
    template <>
    struct __mk_idx_seq<0> { using type = index_sequence<>; };
    template <>
    struct __mk_idx_seq<1> { using type = index_sequence<0>; };

    template<size_t N>
    constexpr typename __mk_idx_seq<N>::type make_index_sequence() noexcept
    { return {}; }
}
#else // __cplusplus
// not even C++11 support
#error "Your C++ compiler must support at least C++11."
#endif // __cplusplus

// FIXME: what is the defined value of compliant C++17 compilers?
#if __cplusplus > 201402L
#include <type_traits>
#else
namespace std {
    /// little helper for the SFINAE idiom we'll use (not required in C++17)
    template<typename... Ts> struct make_void { using type = void;};
    /// little helper for the SFINAE idiom we'll use (not required in C++17)
    template<typename... Ts> using void_t = typename make_void<Ts...>::type;
}
#endif // __cplusplus

#endif // CPP14_COMPAT_H

/* Copyright (C) CERN for the benefit of the LHCb collaboration
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * In applying this licence, CERN does not waive the privileges and immunities
 * granted to it by virtue of its status as an Intergovernmental Organization
 * or submit itself to any jurisdiction.
 */

// vim: sw=4:tw=78:ft=cpp:et
