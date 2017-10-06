/** @file SOASkin.h
 *
 * @author Manuel Schiller <Manuel.Schiller@cern.ch>
 * @date 2017-10-03
 *
 * @brief convenient SOA skins: the ugly mechanics that work under the hood
 */

#ifndef SOASKIN_H
#define SOASKIN_H

#include "SOATypelist.h"
#include "SOAUtils.h"
#include "PrintableNullSkin.h"

/// namespace to encapsulate SOA stuff
namespace SOA {
    /// implementation details for convenient SOA skins
    namespace impl {
        /// little helper checking for duplicate fields
        template <typename... FIELDS>
        struct has_duplicate_fields {
            using TL = SOA::Typelist::typelist<FIELDS...>;
            /// predicate to test if field T is duplicate in FIELDS...
            template <typename T>
            struct type : std::integral_constant<bool,
                (TL::template count<T>() > 1)> {};
        };
        /// little helper checking for duplicate fields
        template <bool HASDUPLICATES, typename BASE, typename... FIELDS>
        struct _NoDuplicateFieldsVerifier {
            static_assert(!HASDUPLICATES, "Duplicate fields are not allowed.");
        };
        /// specialisation: no duplicate fields
        template <typename BASE, typename... FIELDS>
        struct _NoDuplicateFieldsVerifier<false, BASE, FIELDS...> :
            FIELDS::template accessors<BASE>... {};
        /// little helper checking for duplicate fields
        template <typename BASE, typename... FIELDS>
        using NoDuplicateFieldsVerifier = _NoDuplicateFieldsVerifier<
            SOA::Utils::ANY<
            has_duplicate_fields<FIELDS...>::template type,
            FIELDS...>::value, BASE, FIELDS...>;
        /** @brief base class of all convenient SOA skins
         *
         * @author Manuel Schiller <Manuel.Schiller@cern.ch>
         * @date 2017-10-03
         *
         * @tparam BASE         underlying tuple/whatever
         * @tparam FIELDS       fields to which grant access
         */
        template <typename BASE, class... FIELDS>
        struct SkinBase :
            // order important for empty base class optimisation
            NoDuplicateFieldsVerifier<SkinBase<BASE, FIELDS...>, FIELDS...>, BASE
        {
            // make sure that no user puts data in a field...
            static_assert(SOA::Utils::ALL<std::is_empty, FIELDS...>::value,
                    "Fields may not contain data or virtual methods!");
            static_assert(SOA::Utils::ALL<std::is_empty,
                    typename FIELDS::template accessors<BASE>...>::value,
                    "Field accessors may not contain data or virtual methods!");
            /// inform the framework that we're a skin
            using skin_tag = struct {};
            /// we're the base class underlying classes that inherit from us
            using base_type = SkinBase;
            /// typelist listing all fields
            using fields_typelist = SOA::Typelist::typelist<FIELDS...>;
            /// forward to *underlying constructors operators
            using BASE::BASE;
            /// forward to *underlying assignment operators
            using BASE::operator=;
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

// vim: sw=4:tw=78:ft=cpp:et
