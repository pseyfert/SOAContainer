/** @file include/SOAAlgorithms.h
 *
 * @author Manuel Schiller <Manuel.Schiller@glasgow.ac.uk>
 * @date 2019-06-05
 *
 * For copyright and license information, see the end of the file.
 */
#pragma once

#include <type_traits>
#include <tuple>
#include <vector>

#include "c++14_compat.h"
#include "SOATypelist.h"
#include "SOATypelistUtils.h"
#include "SOAUtils.h"
#include "SOATaggedType.h"
#include "SOAContainer.h"

namespace SOA {
    /// namespace with SOA algorithm implementation details
    namespace impl_algs {
        /// helper: gather information on a callable
        template <typename FN>
        struct __callable_info;
        template <typename R, typename... ARGS>
        struct __callable_info<R (*)(ARGS...)> {
            using result_type = R;
            using arg_typelist = SOA::Typelist::typelist<ARGS...>;
        };
        template <typename R, typename FN, typename... ARGS>
        struct __callable_info<R (FN::*)(ARGS...)> {
            using result_type = R;
            using arg_typelist = SOA::Typelist::typelist<ARGS...>;
        };
        template <typename R, typename FN, typename... ARGS>
        struct __callable_info<R (FN::*)(ARGS...) const> {
            using result_type = R;
            using arg_typelist = SOA::Typelist::typelist<ARGS...>;
        };

        /// helper: gather information on a callable
        template <bool ISFN, typename FN>
        struct _callable_info;
        template <typename FN>
        struct _callable_info<true, FN> : __callable_info<FN> {};
        template <typename FN>
        struct _callable_info<false, FN>
                : __callable_info<decltype(&FN::operator())> {};

        /// gather information on a callable (arguments, return value)
        template <typename FN>
        struct callable_info
                : _callable_info<std::is_function<FN>::value, FN> {};

        /// check how often the typelist TL contains ARG (unwrap if needed)
        template <typename TL, typename ARG,
                  bool = SOA::is_tagged_type<ARG>::value>
        struct contains_n;
        template <typename TL, typename ARG>
        struct contains_n<TL, ARG, true>
                : std::integral_constant<
                          std::size_t,
                          TL::template count<typename ARG::field_type>()> {};
        template <typename TL, typename ARG>
        struct contains_n<TL, ARG, false>
                : std::integral_constant<
                          std::size_t,
                          TL::template map_t<SOA::Typelist::unwrap_t>::
                                  template count<ARG>()> {};

        // find ARG in TL (taking care to unwrap if needed)
        template <typename TL, typename ARG,
                  bool = SOA::is_tagged_type<ARG>::value>
        struct find_idx;
        template <typename TL, typename ARG>
        struct find_idx<TL, ARG, true>
                : std::integral_constant<
                          std::size_t,
                          TL::template find<typename ARG::field_type>()> {};
        template <typename TL, typename ARG>
        struct find_idx<TL, ARG, false>
                : std::integral_constant<
                          std::size_t,
                          TL::template map_t<SOA::Typelist::unwrap_t>::
                                  template find<ARG>()> {};

        /// check if all ARGS in a typelist can be found in VIEW
        template <typename VIEW, typename... ARGS>
        constexpr std::integral_constant<
                bool,
                SOA::Utils::ALL(
                        (contains_n<typename VIEW::fields_typelist,
                                    typename std::remove_cv<
                                            typename std::remove_reference<
                                                    ARGS>::type>::type>() >
                         0)...)>
        canFindArgs(const VIEW& /* unused */,
                    SOA::Typelist::typelist<ARGS...> /* unused */) noexcept
        {
            return {};
        }

        /// check if all ARGS in a typelist can be found in VIEW exactly once
        template <typename VIEW, typename... ARGS>
        constexpr std::integral_constant<
                bool,
                SOA::Utils::ALL(
                        (contains_n<typename VIEW::fields_typelist,
                                    typename std::remove_cv<
                                            typename std::remove_reference<
                                                    ARGS>::type>::type>() ==
                         1)...)>
        uniqueArgs(const VIEW& /* unused */,
                   SOA::Typelist::typelist<ARGS...> /* unused */) noexcept
        {
            return {};
        }

        /// turn a single value or a tuple into a typelist
        template <typename... ARGS>
        struct typelist_from_tuple {
            using typelist = SOA::Typelist::typelist<ARGS...>;
            using tuple_type = std::tuple<ARGS...>;
        };
        template <typename... ARGS>
        struct typelist_from_tuple<std::tuple<ARGS...>> {
            using typelist = SOA::Typelist::typelist<ARGS...>;
            using tuple_type = std::tuple<ARGS...>;
        };

        /// check if ARG (or std::tuple<ARGS...>) are all tagged types
        template <typename ARG>
        struct is_type_or_tuple_tagged_type : SOA::is_tagged_type<ARG> {};
        template <typename... ARGS>
        struct is_type_or_tuple_tagged_type<std::tuple<ARGS...>>
                : std::integral_constant<bool,
                                         SOA::Utils::ALL(SOA::is_tagged_type<
                                                         ARGS>::value...)> {};

        /// placeholder when no default skin is given by caller
        template <class SKIN>
        struct DefaultSkin {
            struct default_skin_tag {};
        };

        /// check for placeholder skin
        template <template <class> class SKIN, typename = void>
        struct is_default_skin : std::false_type {};
        template <template <class> class SKIN>
        struct is_default_skin<
                SKIN, std::void_t<typename SKIN<void>::default_skin_tag>>
                : std::true_type {};

        /// debugging: check is_default_skin implementation
        namespace debug_impl {
            template <class>
            struct SomeSkin {};
            static_assert(!is_default_skin<SomeSkin>::value,
                          "bug in is_default_skin");
            static_assert(is_default_skin<DefaultSkin>::value,
                          "bug in is_default_skin");
        } // namespace debug_impl

        /// switch between two skins based on predicate
        template <bool COND, template <class> class SKINTRUE,
                  template <class> class SKINFALSE>
        struct conditional_skin;
        template <template <class> class SKINTRUE,
                  template <class> class SKINFALSE>
        struct conditional_skin<false, SKINTRUE, SKINFALSE> {
            template <class T>
            using type = SKINFALSE<T>;
        };
        template <template <class> class SKINTRUE,
                  template <class> class SKINFALSE>
        struct conditional_skin<true, SKINTRUE, SKINFALSE> {
            template <class T>
            using type = SKINTRUE<T>;
        };

        /// ignore all arguments, do nothing
        template <typename... ARGS>
        void nop(ARGS&&... /* unused */) noexcept
        {}

        /// reserve space in container (if container supports it)
        template <typename C, typename = void>
        struct reserve_if_possible {
            reserve_if_possible(C& /* unused */,
                                typename C::size_type /* unused */)
            {}
        };
        template <typename C>
        struct reserve_if_possible<
                C, std::void_t<decltype(std::declval<C&>().reserve(0))>> {
            reserve_if_possible(C& c, typename C::size_type szneeded)
            {
                c.reserve(c.size() + szneeded);
            }
        };

        /// helper for transform
        template <template <class> class SKIN,
                  template <class...> class CONTAINER, typename VIEW,
                  typename FUNC, std::size_t... IDXS, typename... ARGS,
                  std::size_t... OUTIDXS, typename... OUTARGS>
        SOA::Container<CONTAINER, SKIN, typename OUTARGS::field_type...>
        __transform(std::index_sequence<IDXS...> /* unused */,
                    SOA::Typelist::typelist<ARGS...> /* unused */,
                    std::index_sequence<OUTIDXS...> /* unused */,
                    SOA::Typelist::typelist<OUTARGS...> /* unused */,
                    VIEW&& view, FUNC&& func)
        {
            SOA::Container<CONTAINER, SKIN, typename OUTARGS::field_type...>
                    retVal;
            reserve_if_possible<decltype(retVal)>(retVal, view.size());
            auto its = std::make_tuple(
                    view.template begin<find_idx<
                            typename std::remove_reference<
                                    VIEW>::type::fields_typelist,
                            typename std::remove_cv<
                                    typename std::remove_reference<
                                            ARGS>::type>::type>::value>()...);
            const auto itEnd = std::get<0>(std::make_tuple(
                    view.template end<find_idx<
                            typename std::remove_reference<
                                    VIEW>::type::fields_typelist,
                            typename std::remove_cv<
                                    typename std::remove_reference<ARGS>::
                                            type>::type>::value>()...));
            for (; itEnd != std::get<0>(its);
                 nop((++std::get<IDXS>(its), 0)...)) {
                std::tuple<OUTARGS...> result(
                        std::forward<FUNC>(func)(ARGS(*std::get<IDXS>(its))...));
                retVal.emplace_back(std::move(std::get<OUTIDXS>(result))...);
            }
            return retVal;
        }

        /// helper for transform
        template <template <class> class SKIN,
                  template <class...> class CONTAINER, typename VIEW,
                  typename FUNC, std::size_t... IDXS, typename... ARGS,
                  std::size_t... OUTIDXS, typename... OUTARGS>
        auto _transform(std::index_sequence<IDXS...> /* unused */,
                        SOA::Typelist::typelist<ARGS...> /* unused */,
                        std::index_sequence<OUTIDXS...> /* unused */,
                        SOA::Typelist::typelist<OUTARGS...> /* unused */,
                        VIEW&& view, FUNC&& func)
                -> decltype(__transform<
                            conditional_skin<
                                    is_default_skin<SKIN>::value,
                                    SOA::impl::SOASkinCreatorSimple<
                                            typename OUTARGS::field_type...>::
                                            template type,
                                    SKIN>::template type,
                            CONTAINER>(std::index_sequence<IDXS...>(),
                                       SOA::Typelist::typelist<ARGS...>(),
                                       std::index_sequence<OUTIDXS...>(),
                                       SOA::Typelist::typelist<OUTARGS...>(),
                                       std::forward<VIEW>(view),
                                       std::forward<FUNC>(func)))
        {
            return __transform<
                    conditional_skin<
                            is_default_skin<SKIN>::value,
                            SOA::impl::SOASkinCreatorSimple<
                                    typename OUTARGS::field_type...>::
                                    template type,
                            SKIN>::template type,
                    CONTAINER>(std::index_sequence<IDXS...>(),
                               SOA::Typelist::typelist<ARGS...>(),
                               std::index_sequence<OUTIDXS...>(),
                               SOA::Typelist::typelist<OUTARGS...>(),
                               std::forward<VIEW>(view),
                               std::forward<FUNC>(func));
        }
    } // namespace impl_algs

    /** @brief transform an (SOA) View
     *
     * @param view          view to transform
     * @param func          function/functor that calculates the
     *                      transformation
     * @tparam SKIN         (optional) skin to use for the returned container
     * @tparam CONTAINER    (optional) underlying container type used for
     *                      returned container
     *
     * @returns a SOA::Container with the transformed result
     *
     * Example:
     * @code
     * auto view = get_some_view();
     * auto c = transform(view, [] (SOA::cref<field1> x, SOA::cref<field2> y)
     * { return std::make_tuple(SOA::value<field_sum>(x + y),
     *                             SOA::value<field_prod>(x * y));
     *      });
     * // c is now a container with two fields: field_sum and field_prod
     * @endcode
     *
     * Tagged values in the argument list of the functor/function can be
     * omitted if "naked" data type used in its place is sufficient to
     * uniquely identify which field of the view is meant. For example, if the
     * view contains only one integer field, it's sufficient to only put "int
     * i" in the argument list. If the types of the arguments of the functor
     * are not sufficient to uniquely identify the fields of the view that are
     * required, a compiler error is produced (with a suitable diagnostic
     * message).
     *
     * For the return value of the functor, tagged types (i.e.
     * SOA::value<field>) must be used in all cases. If only a single value is
     * returned, there is no need to put that one value in a tuple.
     */
    template <template <class> class SKIN = SOA::impl_algs::DefaultSkin,
              template <class...> class CONTAINER = std::vector,
              typename VIEW, typename FUNC>
    auto
    transform(VIEW&& view,
              FUNC&& func) -> decltype(SOA::impl_algs::_transform<SKIN,
                                                                  CONTAINER>(
            std::make_index_sequence<SOA::impl_algs::callable_info<
                    FUNC>::arg_typelist::size()>(),
            typename SOA::impl_algs::callable_info<FUNC>::arg_typelist(),
            std::make_index_sequence<SOA::impl_algs::typelist_from_tuple<
                    typename SOA::impl_algs::callable_info<
                            FUNC>::result_type>::typelist::size()>(),
            typename SOA::impl_algs::typelist_from_tuple<
                    typename SOA::impl_algs::callable_info<
                            FUNC>::result_type>::typelist(),
            std::forward<VIEW>(view), std::forward<FUNC>(func)))
    {
        using arg_typelist =
                typename SOA::impl_algs::callable_info<FUNC>::arg_typelist;
        using result_typelist = typename SOA::impl_algs::typelist_from_tuple<
                typename SOA::impl_algs::callable_info<FUNC>::result_type>::
                typelist;
        static_assert(
                decltype(SOA::impl_algs::canFindArgs(
                        std::declval<const VIEW&>(), arg_typelist()))::value,
                "some function arguments not found in view");
        static_assert(
                decltype(SOA::impl_algs::uniqueArgs(
                        std::declval<const VIEW&>(), arg_typelist()))::value,
                "unable to uniquely match all arguments "
                "- try using tagged types as argunents");
        static_assert(SOA::impl_algs::is_type_or_tuple_tagged_type<
                              typename SOA::impl_algs::callable_info<
                                      FUNC>::result_type>::value,
                      "function return type must be tagged type or tuple of "
                      "tagged types");
        return SOA::impl_algs::_transform<SKIN, CONTAINER>(
                std::make_index_sequence<arg_typelist::size()>(),
                arg_typelist(),
                std::make_index_sequence<result_typelist::size()>(),
                result_typelist(), std::forward<VIEW>(view),
                std::forward<FUNC>(func));
    }

    namespace impl_algs {
        /// helper for for_each
        template <typename VIEW, typename FUNC, std::size_t... IDXS,
                  typename... ARGS>
        void _for_each(std::index_sequence<IDXS...> /* unused */,
                        SOA::Typelist::typelist<ARGS...> /* unused */,
                        VIEW&& view, FUNC&& func)
        {
            auto its = std::make_tuple(
                    view.template begin<find_idx<
                            typename std::remove_reference<
                                    VIEW>::type::fields_typelist,
                            typename std::remove_cv<
                                    typename std::remove_reference<
                                            ARGS>::type>::type>::value>()...);
            const auto itEnd = std::get<0>(std::make_tuple(
                    view.template end<find_idx<
                            typename std::remove_reference<
                                    VIEW>::type::fields_typelist,
                            typename std::remove_cv<
                                    typename std::remove_reference<ARGS>::
                                            type>::type>::value>()...));
            for (; itEnd != std::get<0>(its);
                 nop((++std::get<IDXS>(its), 0)...)) {
                std::forward<FUNC>(func)(ARGS(*std::get<IDXS>(its))...);
            }
        }
    }

    /** @brief apply a function to each element of a (SOA) View
     *
     * @param view          view to apply function to
     * @param func          function/functor to apply
     *                      transformation
     *
     * Example:
     * @code
     * auto view = get_some_view();
     * auto c = for_each(view, [] (SOA::ref<field1> x, SOA::ref<field2> y) {
     *         field1::type tmp = x; // auto will deduce SOA::ref<field1>,
     *                               // which is not what we want - we need
     *                               // a copy
     *         x = 2 * y;
     *         y = tmp;
     *     });
     * // c is modified in place
     * @endcode
     *
     * Tagged values in the argument list of the functor/function can be
     * omitted if "naked" data type used in its place is sufficient to
     * uniquely identify which field of the view is meant. For example, if the
     * view contains only one integer field, it's sufficient to only put "int
     * i" in the argument list. If the types of the arguments of the functor
     * are not sufficient to uniquely identify the fields of the view that are
     * required, a compiler error is produced (with a suitable diagnostic
     * message).
     */
    template <typename VIEW, typename FUNC>
    void for_each(VIEW&& view, FUNC&& func)
    {
        using arg_typelist =
                typename SOA::impl_algs::callable_info<FUNC>::arg_typelist;
        static_assert(
                decltype(SOA::impl_algs::canFindArgs(
                        std::declval<const VIEW&>(), arg_typelist()))::value,
                "some function arguments not found in view");
        static_assert(
                decltype(SOA::impl_algs::uniqueArgs(
                        std::declval<const VIEW&>(), arg_typelist()))::value,
                "unable to uniquely match all arguments "
                "- try using tagged types as argunents");

        SOA::impl_algs::_for_each(
                std::make_index_sequence<arg_typelist::size()>(),
                arg_typelist(), std::forward<VIEW>(view),
                std::forward<FUNC>(func));
    }
} // namespace SOA

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

// vim: sw=4:ft=cpp:et:tw=78
