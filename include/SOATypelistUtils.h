/** @file SOATypelistUtils.h
 *
 * @author Manuel Schiller <Manuel.Schiller@cern.ch>
 * @date 2015-04-10
 *
 * For copyright and license information, see the end of the file.
 */

#ifndef SOATYPELISTUTILS_H
#define SOATYPELISTUTILS_H

#include <tuple>
#include <deque>
#include <vector>

#include "SOATypelist.h"
#include "AlignedAllocator.h"
#include "c++14_compat.h"

/// namespace to encapsulate SOA stuff
namespace SOA {
    namespace Typelist {
        /// is a type a wrapped type or not (see below)?
        template <typename T, typename = void>
        struct is_wrapped : std::false_type {};
        /// is a type a wrapped type or not (see below)?
        template <typename T>
        struct is_wrapped<T, std::void_t<typename T::wrap_tag> > :
                std::true_type {};

        /// type to "wrap" other types (to  "distinguish" instances of same type)
        template<typename T, bool DUMMY = is_wrapped<T>::value> struct wrap_type;
        /// specialisation: wrapping a wrap_type results in the type itself
        template<typename T> struct wrap_type<T, true>
        { using wrap_tag = void; using type = typename T::type; };
        /// specialisation: wrap a type
        template<typename T> struct wrap_type<T, false>
        { using wrap_tag = void; using type = T; };
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
            using xAtYEq0 = struct xAtYEq0 : public wrap_type<double> {};
            using zAtYEq0 = struct zAtYEq0 : public wrap_type<double> {};
            using y = struct y : public wrap_type<double> {};
            using dxdy = struct dxdy : public wrap_type<double> {};
            using dzdy = struct dzdy : public wrap_type<double> {};

            using hitfields = typelist<xAtYEq0, zAtYEq0, y, dxdy, dzdy>;
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

        /// implementation details for to_tuple below
        namespace _to_tuple_impl {
            /// select the concrete container type
            template <typename T, template <typename...> class CONTAINER>
            struct select_concrete_container {
                using _t = CONTAINER<T>;
            };
            /* specialisations: std::vectors and std::deque
             * allocations are cache-line aligned by default
             *
             * pity deque's iterators are too complex to be understood by the
             * compiler in most STL implementations I've seen...
             */
            template <typename T>
            struct select_concrete_container<T, std::vector> {
                using _t = std::vector<T, CacheLineAlignedAllocator<T> >;
            };
            template <typename T>
            struct select_concrete_container<T, std::deque>
            {
#if __cplusplus >= 201402L // be nice an give users a hint
                // a deque-like data structure to back the SOA::Container
                // or SOA::View is not a bad idea in principle, but I haven't
                // seen good code from current implementations - maybe
                // somebody writes a custom deque which the compiler will
                // basically optimize away...
                using _t [[deprecated(
                        "std::deque performance is bad in all STL "
                        "implementations I have seen (libstdc++, libc++), "
                        "since the compiler doesn't seem to optimize out the "
                        "iterators, and hence does not vectorize")]] =
                        std::deque<T, CacheLineAlignedAllocator<T>>;
#else
                using _t = std::deque<T, CacheLineAlignedAllocator<T>>;
#endif
            };
        }

        /// class to give tuple types based on TL's listed types
        template <typename TL>
        class to_tuple {
            private:
                template <typename T> using decay_t = typename std::decay<T>::type;
                template <template <typename...> class CONTAINER = std::vector>
                struct container_of {
                    template <typename T> using _t = typename _to_tuple_impl::select_concrete_container<T, CONTAINER>::_t;
                };
            public:
                template <typename... ARGS>
                static std::tuple<ARGS...> to_tuple_fn(typelist<ARGS...>) noexcept;
                template <typename... ARGS>
                static std::tuple<ARGS&...> to_ref_tuple_fn(typelist<ARGS...>) noexcept;
                template <typename... ARGS>
                static std::tuple<const ARGS&...> to_cref_tuple_fn(typelist<ARGS...>) noexcept;
                template <typename... ARGS>
                static std::tuple<ARGS&&...> to_rval_tuple_fn(typelist<ARGS...>) noexcept;
                using value_tuple = decltype(to_tuple_fn(typename TL::template map_t<unwrap_t>::template map_t<decay_t>()));
                using rvalue_tuple = decltype(to_rval_tuple_fn(typename TL::template map_t<unwrap_t>::template map_t<decay_t>()));
                using reference_tuple = decltype(to_ref_tuple_fn(typename TL::template map_t<unwrap_t>::template map_t<decay_t>()));
                using const_reference_tuple = decltype(to_cref_tuple_fn(typename TL::template map_t<unwrap_t>::template map_t<decay_t>()));
                template <template <typename...> class CONTAINER = std::vector>
                using container_tuple = decltype(to_tuple_fn(typename TL::template map_t<unwrap_t>::template map_t<container_of<CONTAINER>::template _t>()));
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
            static_assert(std::is_same<
                    typename to_tuple<typelist<int, float> >::template container_tuple<std::vector>,
                    std::tuple<std::vector<int, CacheLineAlignedAllocator<int> >,
                        std::vector<float, CacheLineAlignedAllocator<float> > > >::value,
                    "implementation error");
        }
    } // namespace Typelist
} // namespace SOA

#endif // SOATYPELISTUTILS_H

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
