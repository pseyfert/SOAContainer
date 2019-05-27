/** @file include/SOATaggedType.h
 *
 * @author Manuel Schiller <Manuel.Schiller@glasgow.ac.uk>
 * @date 2019-05-21
 *
 * For copyright and license information, see the end of the file.
 */
#pragma once

#include <tuple>
#include <type_traits>
#include <utility>

#include "SOATypelist.h"
#include "SOATypelistUtils.h"
#include "c++14_compat.h"

namespace SOA {
    /// implementation details
    namespace detail {
        /// little class wrapping the data type specified in a field
        template <typename T, typename FIELD>
        class payload : public std::tuple<T> {
            static_assert(SOA::Typelist::is_wrapped<FIELD>::value,
                          "FIELD must be a field");

        public:
            /// type of underlying value
            using value_type = typename std::remove_reference<
                    typename std::remove_cv<T>::type>::type;
            /// type of reference to underlying value
            using reference = value_type&;
            /// type of const reference to underlying value
            using const_reference = const value_type&;
            /// type of field
            using field_type = FIELD;
            /// give a tag so we can recognise a tagged type
            using tagged_type_tag = void;

        protected:
            /// helper to enable/disable the right overloads
            template <typename S>
            using is_const =
                    std::is_const<typename std::remove_reference<S>::type>;
            /// helper to enable/disable the right overloads
            template <typename S>
            using is_arith = typename std::conditional<
                            std::is_arithmetic<value_type>::value &&
                            !is_const<T>::value &&
                            std::is_arithmetic<S>::value &&
                            std::is_convertible<S, value_type>::value,
                    std::true_type, std::false_type>::type;
            /// helper to enable/disable the right overloads
            template <typename S>
            using is_iarith = typename std::conditional<
                    is_arith<S>::value &&
                            std::is_integral<value_type>::value &&
                            std::is_integral<S>::value,
                    std::true_type, std::false_type>::type;

        public:

            /// make the field accessors happy
            template <std::size_t N, typename S = T>
            typename std::enable_if<!is_const<S>::value, reference>::type get() noexcept
            { return std::get<N>(*this); }
            /// make the field accessors happy
            template <std::size_t N, typename S = T>
            const_reference get() const noexcept
            { return std::get<N>(*this); }

            // forward to tuple's constructor/assignment operator
            using std::tuple<T>::tuple;
            using std::tuple<T>::operator=;
            /// converting constructor
            template <typename S,
                      typename = typename std::enable_if<
                              std::is_convertible<S, value_type>::value &&
                              !std::is_same<S, value_type>::value &&
                              !std::is_reference<T>::value>::type>
            payload(const S& payload) : std::tuple<T>{value_type(payload)}
            {}
            /// copy assignment
            template <typename DUMMY = payload>
            typename std::enable_if<std::is_same<DUMMY, payload>::value &&
                                            !is_const<T>::value,
                                    DUMMY>::type&
            operator=(const_reference value)
            {
                std::get<0>(&this) = value;
                return *this;
            }
            /// move assignment
            template <typename DUMMY = payload>
            typename std::enable_if<std::is_same<DUMMY, payload>::value &&
                                            !is_const<T>::value,
                                    DUMMY>::type&
            operator=(value_type&& value)
            {
                std::get<0>(*this) = std::move(value);
                return *this;
            }
            /// copy assignment (potentially converts)
            template <typename S, typename FIELD2>
            typename std::enable_if<
                    !is_const<T>::value &&
                            !std::is_same<value_type, S>::value &&
                            std::is_convertible<S, T>::value,
                    payload>::type&
            operator=(const payload<S, FIELD2>& value)
            {
                std::get<0>(*this) = value;
                return *this;
            }
            /// move assignment
            template <typename S = value_type, typename FIELD2>
            typename std::enable_if<
                    !is_const<T>::value && std::is_same<value_type, S>::value,
                    payload>::type&
            operator=(payload<S, FIELD2>&& value)
            {
                std::get<0>(*this) = std::move(value.*this);
                return *this;
            }
            /// convert to const_reference
            operator const_reference() const noexcept { return std::get<0>(*this); }
            /// convert to reference (if allowed)
            template <
                    typename S = T,
                    typename DUMMY = typename std::enable_if<
                            std::is_same<S, T>::value && !is_const<S>::value,
                            reference>::type>
            operator DUMMY()
            {
                return std::get<0>(*this);
            }
            /// operator+= (if allowed)
            template <typename S>
            typename std::enable_if<is_arith<S>::value, payload>::type&
            operator+=(const S& other)
            {
                std::get<0>(*this) += other;
                return *this;
            }
            /// operator-= (if allowed)
            template <typename S>
            typename std::enable_if<is_arith<S>::value, payload>::type&
            operator-=(const S& other)
            {
                std::get<0>(*this) -= other;
                return *this;
            }
            /// operator*= (if allowed)
            template <typename S>
            typename std::enable_if<is_arith<S>::value, payload>::type&
            operator*=(const S& other)
            {
                std::get<0>(*this) *= other;
                return *this;
            }
            /// operator/= (if allowed)
            template <typename S>
            typename std::enable_if<is_arith<S>::value, payload>::type&
            operator/=(const S& other)
            {
                std::get<0>(*this) /= other;
                return *this;
            }
            /// operator%= (if allowed)
            template <typename S>
            typename std::enable_if<is_iarith<S>::value, payload>::type&
            operator%=(const S& other)
            {
                std::get<0>(*this) %= other;
                return *this;
            }
            /// operator>>= (if allowed)
            template <typename S>
            typename std::enable_if<is_iarith<S>::value, payload>::type&
            operator>>=(const S& other)
            {
                std::get<0>(*this) >>= other;
                return *this;
            }
            /// operator<<= (if allowed)
            template <typename S>
            typename std::enable_if<is_iarith<S>::value, payload>::type&
            operator<<=(const S& other)
            {
                std::get<0>(*this) <<= other;
                return *this;
            }
            /// operator&= (if allowed)
            template <typename S>
            typename std::enable_if<is_iarith<S>::value, payload>::type&
            operator&=(const S& other)
            {
                std::get<0>(*this) &= other;
                return *this;
            }
            /// operator|= (if allowed)
            template <typename S>
            typename std::enable_if<is_iarith<S>::value, payload>::type&
            operator|=(const S& other)
            {
                std::get<0>(*this) |= other;
                return *this;
            }
            /// operator^= (if allowed)
            template <typename S>
            typename std::enable_if<is_iarith<S>::value, payload>::type&
            operator^=(const S& other)
            {
                std::get<0>(*this) ^= other;
                return *this;
            }
            /// pre-increment operator (if allowed)
            template <typename S = T,
                      typename DUMMY = typename std::enable_if<
                              std::is_arithmetic<value_type>::value &&
                                      !is_const<S>::value,
                              payload&>::type>
            DUMMY operator++()
            {
                ++std::get<0>(*this);
                return *this;
            }
            /// pre-decrement operator (if allowed)
            template <typename S = T,
                      typename DUMMY = typename std::enable_if<
                              std::is_arithmetic<value_type>::value &&
                                      !is_const<S>::value,
                              payload&>::type>
            DUMMY operator--()
            {
                --std::get<0>(*this);
                return *this;
            }
            /// post-increment operator (if allowed)
            template <typename S = T,
                      typename DUMMY = typename std::enable_if<
                              std::is_arithmetic<value_type>::value &&
                                      !is_const<S>::value,
                              value_type>::type>
            DUMMY operator++(int /* unused */)
            {
                value_type retVal = std::get<0>(*this);
                ++std::get<0>(*this);
                return retVal;
            }
            /// post-decrement operator (if allowed)
            template <typename S = T,
                      typename DUMMY = typename std::enable_if<
                              std::is_arithmetic<value_type>::value &&
                                      !is_const<S>::value,
                              value_type>::type>
            DUMMY operator--(int /* unused */)
            {
                value_type retVal = std::get<0>(*this);
                --std::get<0>(*this);
                return retVal;
            }
        };

        /// helper for binary operators of two payloads
        template <typename S, typename T, typename FIELD1, typename FIELD2,
                  typename RESULT = typename std::common_type<
                          typename payload<S, FIELD1>::value_type,
                          typename payload<T, FIELD2>::value_type>::type>
        using retval_if_is_arith = typename std::enable_if<
                std::is_arithmetic<
                        typename payload<S, FIELD1>::value_type>::value &&
                        std::is_arithmetic<typename payload<
                                T, FIELD2>::value_type>::value,
                RESULT>;

        /// helper for binary operators of two payloads
        template <typename S, typename T, typename FIELD1, typename FIELD2,
                  typename RESULT = typename std::common_type<
                          typename payload<S, FIELD1>::value_type,
                          typename payload<T, FIELD2>::value_type>::type>
        using retval_if_is_iarith = typename std::enable_if<
                std::is_arithmetic<
                        typename payload<S, FIELD1>::value_type>::value &&
                        std::is_arithmetic<typename payload<
                                T, FIELD2>::value_type>::value &&
                        std::is_integral<typename payload<
                                S, FIELD1>::value_type>::value &&
                        std::is_integral<typename payload<
                                T, FIELD2>::value_type>::value,
                RESULT>;

        template <typename S, typename T, typename FIELD1, typename FIELD2>
        typename retval_if_is_arith<S, T, FIELD1, FIELD2>::type
        operator+(const payload<S, FIELD1>& a, const payload<T, FIELD2>& b)
        {
            return a.operator typename payload<S, FIELD1>::const_reference() +
                   b.operator typename payload<T, FIELD2>::const_reference();
        }
        template <typename S, typename T, typename FIELD1, typename FIELD2>
        typename retval_if_is_arith<S, T, FIELD1, FIELD2>::type
        operator-(const payload<S, FIELD1>& a, const payload<T, FIELD2>& b)
        {
            return a.operator typename payload<S, FIELD1>::const_reference() -
                   b.operator typename payload<T, FIELD2>::const_reference();
        }
        template <typename S, typename T, typename FIELD1, typename FIELD2>
        typename retval_if_is_arith<S, T, FIELD1, FIELD2>::type
        operator*(const payload<S, FIELD1>& a, const payload<T, FIELD2>& b)
        {
            return a.operator typename payload<S, FIELD1>::const_reference() *
                   b.operator typename payload<T, FIELD2>::const_reference();
        }
        template <typename S, typename T, typename FIELD1, typename FIELD2>
        typename retval_if_is_arith<S, T, FIELD1, FIELD2>::type
        operator/(const payload<S, FIELD1>& a, const payload<T, FIELD2>& b)
        {
            return a.operator typename payload<S, FIELD1>::const_reference() /
                   b.operator typename payload<T, FIELD2>::const_reference();
        }
        template <typename S, typename T, typename FIELD1, typename FIELD2>
        typename retval_if_is_iarith<S, T, FIELD1, FIELD2>::type
        operator%(const payload<S, FIELD1>& a, const payload<T, FIELD2>& b)
        {
            return a.operator typename payload<S, FIELD1>::const_reference() %
                   b.operator typename payload<T, FIELD2>::const_reference();
        }
        template <typename S, typename T, typename FIELD1, typename FIELD2>
        typename retval_if_is_iarith<S, T, FIELD1, FIELD2>::type
        operator>>(const payload<S, FIELD1>& a, const payload<T, FIELD2>& b)
        {
            return a.
                   operator typename payload<S, FIELD1>::const_reference() >>
                   b.operator typename payload<T, FIELD2>::const_reference();
        }
        template <typename S, typename T, typename FIELD1, typename FIELD2>
        typename retval_if_is_iarith<S, T, FIELD1, FIELD2>::type
        operator<<(const payload<S, FIELD1>& a, const payload<T, FIELD2>& b)
        {
            return a.operator typename payload<S, FIELD1>::const_reference()
                   << b.
                      operator typename payload<T, FIELD2>::const_reference();
        }
        template <typename S, typename T, typename FIELD1, typename FIELD2>
        typename retval_if_is_iarith<S, T, FIELD1, FIELD2>::type
        operator&(const payload<S, FIELD1>& a, const payload<T, FIELD2>& b)
        {
            return a.operator typename payload<S, FIELD1>::const_reference() &
                   b.operator typename payload<T, FIELD2>::const_reference();
        }
        template <typename S, typename T, typename FIELD1, typename FIELD2>
        typename retval_if_is_iarith<S, T, FIELD1, FIELD2>::type
        operator|(const payload<S, FIELD1>& a, const payload<T, FIELD2>& b)
        {
            return a.operator typename payload<S, FIELD1>::const_reference() |
                   b.operator typename payload<T, FIELD2>::const_reference();
        }
        template <typename S, typename T, typename FIELD1, typename FIELD2>
        typename retval_if_is_iarith<S, T, FIELD1, FIELD2>::type
        operator^(const payload<S, FIELD1>& a, const payload<T, FIELD2>& b)
        {
            return a.operator typename payload<S, FIELD1>::const_reference() ^
                   b.operator typename payload<T, FIELD2>::const_reference();
        }
    } // namespace detail

    /// encapsulate a value to type specified in field
    template <typename FIELD>
    struct value : FIELD::template accessors<
                           value<FIELD>, SOA::Typelist::typelist<FIELD>,
                           SOA::detail::payload<typename FIELD::type, FIELD>>,
                   SOA::detail::payload<typename FIELD::type, FIELD> {
        using SOA::detail::payload<typename FIELD::type, FIELD>::payload;
        using SOA::detail::payload<typename FIELD::type, FIELD>::operator=;
        /// give a tag so we can recognise a tagged value
        using tagged_value_tag = void;
    };
    /// encapsulate a reference to type specified in field
    template <typename FIELD>
    struct ref : FIELD::template accessors<
                         ref<FIELD>, SOA::Typelist::typelist<FIELD>,
                         SOA::detail::payload<typename FIELD::type&, FIELD>>,
                 SOA::detail::payload<typename FIELD::type&, FIELD> {
        using SOA::detail::payload<typename FIELD::type&, FIELD>::payload;
        using SOA::detail::payload<typename FIELD::type&, FIELD>::operator=;
        /// give a tag so we can recognise a tagged value
        using tagged_reference_tag = void;
    };
    /// encapsulate a const reference to type specified in field
    template <typename FIELD>
    struct cref : FIELD::template accessors<
                          cref<FIELD>, SOA::Typelist::typelist<FIELD>,
                          SOA::detail::payload<const typename FIELD::type&,
                                               FIELD>>,
                  SOA::detail::payload<const typename FIELD::type&, FIELD> {
        using SOA::detail::payload<const typename FIELD::type&, FIELD>::payload;
        using SOA::detail::payload<const typename FIELD::type&, FIELD>::operator=;
        /// give a tag so we can recognise a tagged value
        using tagged_const_reference_tag = void;
    };

    /// check if T is a tagged type
    template <typename T, typename = void>
    struct is_tagged_type : std::false_type {};
    template <typename T>
    struct is_tagged_type<T, std::void_t<typename T::tagged_type_tag>>
            : std::true_type {};

    /// check if T is a tagged value
    template <typename T, typename = void>
    struct is_tagged_value : std::false_type {};
    template <typename T>
    struct is_tagged_value<T, std::void_t<typename T::tagged_value_tag>>
            : std::true_type {};

    /// check if T is a tagged reference
    template <typename T, typename = void>
    struct is_tagged_reference : std::false_type {};
    template <typename T>
    struct is_tagged_reference<T,
                               std::void_t<typename T::tagged_reference_tag>>
            : std::true_type {};

    /// check if T is a tagged const reference
    template <typename T, typename = void>
    struct is_tagged_const_reference : std::false_type {};
    template <typename T>
    struct is_tagged_const_reference<
            T, std::void_t<typename T::tagged_const_reference_tag>>
            : std::true_type {};

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

// vim: sw=4:tw=78:ft=cpp:et