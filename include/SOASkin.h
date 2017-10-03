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
#include "PrintableNullSkin.h"

/// implementation details for convenient SOA skins
namespace SOASkin_impl {
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
        FIELDS::template accessors<SkinBase<BASE, FIELDS...> >..., BASE
    {
        // make sure that no user puts data in a field...
        static_assert(SOAUtils::ALL<std::is_empty, FIELDS...>::value,
                "Fields may not contain data or virtual methods!");
        static_assert(SOAUtils::ALL<std::is_empty,
                typename FIELDS::template accessors<BASE>...>::value,
                "Field accessors may not contain data or virtual methods!");
        /// inform the framework that we're a skin
        using skin_tag = struct {};
        /// we're the base class underlying classes that inherit from us
        using base_type = SkinBase;
        /// typelist listing all fields
        using fields_typelist = SOATypelist::typelist<FIELDS...>;
        /// forward to *underlying constructors operators
        using BASE::BASE;
        /// forward to *underlying assignment operators
        using BASE::operator=;
    };
    /// little helper to detect the base type of a skin (saves typing)
    template <typename T>
    using detect_skin_base = typename T::base_type;
}

/** @brief macro to forward default constructors/assignment operators for a skin
 *
 * @author Manuel Schiller <Manuel.Schiller@cern.ch>
 * @date 2017-10-03
 *
 * @param name 	name of the skin class
 *
 * For a usage example, please see SOASKIN_CUSTOM below.
 */
#define SOASKIN_DEFAULT_CONSTRUCTORS_AND_ASSIGNMENTOPERATORS(name) \
    using __BASE__ = SOASkin_impl::detect_skin_base<name>; \
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
 * SOAFIELD(x, float);
 * SOAFIELD(y, float);
 * // define a SOA skin class with these fields
 * SOASKIN_CUSTOM(SOAPoint, f_x, f_y) {
 *     // we want the default set of constructors and assignment operators
 *     SOASKIN_DEFAULT_CONSTRUCTORS_AND_ASSIGNMENTOPERATORS(SOAPoint);
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
#define SOASKIN_CUSTOM(name, ... /* fields */) \
    template <class BASE> struct name : \
        SOASkin_impl::SkinBase<PrintableNullSkin<BASE>, __VA_ARGS__>

/** @brief define a skin which is just the sum of its fields
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
 * SOAFIELD(x, float);
 * SOAFIELD(y, float);
 * // define a SOA skin class with these fields, and nothing else
 * SOASKIN(SOAPointSimple, f_x, f_y);
 * @endcode
 */
#define SOASKIN(name, ... /* fields */) \
    SOASKIN_CUSTOM(name, __VA_ARGS__) { \
        SOASKIN_DEFAULT_CONSTRUCTORS_AND_ASSIGNMENTOPERATORS(name); \
    }

#endif // SOASKIN_H

// vim: sw=4:tw=78:ft=cpp:et
