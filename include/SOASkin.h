/** @file SOASkin.h
 *
 * @author Manuel Schiller <Manuel.Schiller@cern.ch>
 * @date 2017-10-03
 *
 * @brief convenient SOA skins: the ugly mechanics that work under the hood
 *
 * For copyright and license information, see the end of the file.
 */

#ifndef SOASKIN_H
#define SOASKIN_H

#include "SOATypelist.h"
#include "SOATaggedType.h"
#include "SOAUtils.h"
#include "PrintableNullSkin.h"

/// namespace to encapsulate SOA stuff
namespace SOA {
    /// implementation details for convenient SOA skins
    namespace impl {
        /// a dummy struct to stand in for the base class of a skin
        struct dummy { template <std::size_t> void get() const noexcept {} };

        /// check if there are duplicates in fields
        template <typename... FIELDS>
        constexpr static inline bool has_duplicate_fields()
        {
            using TL = SOA::Typelist::typelist<FIELDS...>;
            return SOA::Utils::ANY((TL::template count<FIELDS>() > 1)...);
        }
        /// little helper checking for duplicate fields
        template <bool HASDUPLICATES, typename BASE, typename BASEBASE, typename... FIELDS>
        struct _NoDuplicateFieldsVerifier {
            static_assert(!HASDUPLICATES, "Duplicate fields are not allowed.");
        };
        /// specialisation: no duplicate fields
        template <typename BASE, typename BASEBASE, typename... FIELDS>
        struct _NoDuplicateFieldsVerifier<false, BASE, BASEBASE, FIELDS...> :
            FIELDS::template accessors<BASE, SOA::Typelist::typelist<FIELDS...>, BASEBASE>... {};
        /// little helper checking for duplicate fields
        template <typename BASE, typename BASEBASE, typename... FIELDS>
        using NoDuplicateFieldsVerifier = _NoDuplicateFieldsVerifier<
            SOA::Utils::ANY((SOA::Typelist::typelist<FIELDS...>::template count<FIELDS>() > 1)...),
            BASE, BASEBASE, FIELDS...>;
        /** @brief base class of all convenient SOA skins
         *
         * @author Manuel Schiller <Manuel.Schiller@cern.ch>
         * @date 2017-10-03
         *
         * @tparam BASE         underlying tuple/whatever
         * @tparam FIELDS       fields to which grant access
         */
        template <typename BASE, class... FIELDS>
        class SkinBase :
                // order important for empty base class optimisation
                public NoDuplicateFieldsVerifier<SkinBase<BASE, FIELDS...>,
                                                 BASE, FIELDS...>,
                public BASE
        {
        public:
            // make sure that no user puts data in a field...
            static_assert(SOA::Utils::ALL(std::is_empty<FIELDS>::value...),
                    "Fields may not contain data or virtual methods!");
            static_assert(SOA::Utils::ALL(std::is_empty<
                    typename FIELDS::template accessors<SkinBase, SOA::Typelist::typelist<FIELDS...>, BASE> >::value...),
                    "Field accessors may not contain data or virtual methods!");
            /// inform the framework that we're a skin
            using skin_tag = void;
            /// we're the base class underlying classes that inherit from us
            using base_type = SkinBase;
            /// typelist listing all fields
            using fields_typelist = SOA::Typelist::typelist<FIELDS...>;

            // make compiler provide the whole set of usual constructors and
            // assignment operators (this is ugly like hell, but old compiler
            // versions get confused in complicated code if they're not there)
            SkinBase() = default;
            SkinBase(const SkinBase&) = default;
            SkinBase(SkinBase&&) = default;
            SkinBase& operator=(const SkinBase&) = default;
            SkinBase& operator=(SkinBase&&) = default;

            /// forward to *underlying constructors operators
            template <typename... ARGS,
                      typename = typename std::enable_if<
                              std::is_constructible<BASE, ARGS...>::value &&
                              SOA::Utils::ALL(!SOA::is_tagged_type<
                                              typename std::remove_reference<
                                                      ARGS>::type>::
                                                      value...)>::type>
            SkinBase(ARGS&&... args) noexcept(
                    noexcept(BASE(std::forward<ARGS>(args)...)))
                    : BASE(std::forward<ARGS>(args)...)
            {}

            /// forward to *underlying assignment operators
            template <typename ARG>
            SkinBase& operator=(ARG&& arg) noexcept(
                    noexcept(std::declval<BASE&>().operator=(std::forward<ARG>(arg))))
            {
                BASE::operator=(std::forward<ARG>(arg));
                return *this;
            }

            /** @brief construct from tagged data types
             */
            template <typename... FIELDS2, typename = void,
                      typename = typename std::enable_if<
                              sizeof...(FIELDS2) == sizeof...(FIELDS) &&
                              SOA::Utils::ALL(SOA::is_tagged_type<
                                              typename std::remove_reference<
                                                      FIELDS2>::type>::
                                                      value...) &&
                              SOA::Utils::ALL(
                                      (-std::size_t(1) !=
                                       SOA::Typelist::typelist<
                                               typename std::remove_reference<
                                                       FIELDS2>::type::
                                                       field_type...>::
                                               template find<FIELDS>())...)>::
                              type>
            explicit SkinBase(FIELDS2&&... args) noexcept(
                    noexcept(BASE(SOA::impl::permute_tagged<fields_typelist>(
                            std::forward<FIELDS2>(args)...))))
                    : BASE(SOA::impl::permute_tagged<fields_typelist>(
                              std::forward<FIELDS2>(args)...))
            {}
        };

        /// little helper to detect the base type of a skin (saves typing)
        template <typename T>
        using detect_skin_base = typename T::base_type;

        /** @brief create a simple SOA skin
         *
         * @author Manuel Schiller <Manuel.Schiller@cern.ch>
         * @date 2017-10-04
         *
         * @tparam FIELDS...    fields that the new skin should have
         *
         * The class defines a type that represents the desired simple SOA skin.
         * "Simple" refers to the fact that the skin does not provide any user-defined
         * methods in addition to those inherited from the fields.
         *
         * Usage example:
         * @code
         * // define fields f_x and f_y with accessors x() and y()
         * SOAFIELD(x, float);
         * SOAFIELD(y, float);
         * // define a SOA skin class with these fields, and nothing else
         * template <class BASE>
         * using SOAPointSimple = SOASkinCreatorSimple<f_x, f_y>::type;
         * @endcode
         */
        template <class... FIELDS>
        struct SOASkinCreatorSimple {
            /// type representing the desired skin
            template <class BASE>
            struct type : SkinBase<SOA::PrintableNullSkin<BASE>, FIELDS...> {
                using __BASE__ = detect_skin_base<type>;
                using __BASE__::__BASE__;
                using __BASE__::operator=;
            };
        };
        // helper for is_skin for templates: false in general
        template <template <class...> class T, typename = void>
        struct _is_skin : std::false_type {};
        // helper for is_skin for templates: true if it has a skin_tag
        template <template <class> class T>
        struct _is_skin<T, std::void_t<typename T<dummy>::skin_tag> > :
                std::true_type {};
        /// is a type a skin or not (for templated types)
        template <template <class...> class T>
        constexpr bool is_skin() noexcept { return _is_skin<T>::value; }
        /// is a type a skin or not (non-templated types are never skins)
        template <typename... T>
        constexpr bool is_skin() noexcept { return false; }
    } // impl
} // SOA

/** @brief macro to "inherit" default constructors/assignment operators from
 * underlying tuple.
 *
 * @author Manuel Schiller <Manuel.Schiller@cern.ch>
 * @date 2017-10-03
 *
 * @param name 	name of the skin class
 *
 * For a usage example, please see SOASKIN below. Usually, you will want to
 * use this...
 */
#define SOASKIN_INHERIT_DEFAULT_METHODS(name) \
    using __BASE__ = SOA::impl::detect_skin_base<name>; \
    using __BASE__::__BASE__; using __BASE__::operator=

/** @brief define a skin with custom methods
 *
 * @author Manuel Schiller <Manuel.Schiller@cern.ch>
 * @date 2017-10-03
 *
 * @param name 	name of the SOA skin to be defined
 * @param ... 	fields that the skin "contains"
 *
 * For example, to declare a SOAPoint skin which has two float fields, x
 * and y, and additionally allows the calculation of a radius, one could use
 * this code:
 * @code
 * // define fields f_x and f_y with accessors x() and y()
 * SOAFIELD_TRIVIAL(f_x, x, float);
 * SOAFIELD_TRIVIAL(f_y, y, float);
 * // define a SOA skin class with these fields
 * SOASKIN(SOAPoint, f_x, f_y) {
 *     // we want the default set of constructors and assignment operators
 *     SOASKIN_INHERIT_DEFAULT_METHODS(SOAPoint);
 *     // we don't need any other constructors/assignment operators,
 *     // otherwise we would define them here...
 *
 *     // we inherit the accessors for x and y from the fields automatically
 *
 *     // add a routine to calculate the radial distance from the origin
 *     float r() const
 *     { return std::sqrt(this->x() * this->x() + this->y() * this->y()); }
 * };
 * @endcode
 */
#define SOASKIN(name, ... /* fields */) \
    template <class BASE> struct name : \
        SOA::impl::SkinBase<SOA::PrintableNullSkin<BASE>, __VA_ARGS__>

/** @brief define a trivial skin which is just the sum of its fields
 *
 * @author Manuel Schiller <Manuel.Schiller@cern.ch>
 * @date 2017-10-03
 *
 * @param name 	name of the SOA skin to be defined
 * @param ... 	fields that the skin "contains"
 *
 * For example, to declare a SOAPoint skin which has two float fields, x
 * and y, one could use this code:
 * @code
 * // define fields f_x and f_y with accessors x() and y()
 * SOAFIELD_TRIVIAL(f_x, x, float);
 * SOAFIELD_TRIVIAL(f_y, y, float);
 * // define a SOA skin class with these fields, and nothing else
 * SOASKIN_TRIVIAL(SOAPointSimple, f_x, f_y);
 * @endcode
 */
#define SOASKIN_TRIVIAL(name, ... /* fields */) \
    template <class BASE> \
    using name = SOA::impl::SOASkinCreatorSimple<__VA_ARGS__>::type<BASE>

#endif // SOASKIN_H

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
