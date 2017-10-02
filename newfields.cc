#include <tuple>
#include <vector>
#include <iostream>
#include "include/SOATypelist.h"
#include "include/SOAContainer.h"
#include "include/PrintableNullSkin.h"

/* library code  */

/** @brief class from which a field tag derives
 *
 * @tparam T            type of the field
 * @tparam FIELD        field (child) type to use
 * @tparam SKIN         skin to which we're bound
 */
template <typename T, template <typename> class FIELD, typename SKIN>
class FieldBase {
    public:
        /// the field tag itself
        using self_type = FieldBase;
        /// tell SOAContainer/View that we wrap a type
        using wrap_tag = struct {};
        static_assert(!SOATypelist::is_wrapped<T>::value,
                "Error: Do no nest FieldBase<FieldBase<T>, FIELD, SKIN>!");
        /// type we wrap
        using type = T;
        /// support rebinding to a different skin type
        template <typename NEWSKIN> using rebind = FIELD<NEWSKIN>;
    protected:
        /// retrieve reference to the field
        type& _get()
        {
            return reinterpret_cast<SKIN&>(*this).template
                get<FIELD<void> >();
        }
        /// retrieve const reference to the field
        const type& _get() const
        {
            return reinterpret_cast<const SKIN&>(*this).template
                get<FIELD<void> >();
        }
};

/** @brief base class of all skins
 *
 * @tparam BASE         underlying tuple/whatever
 * @tparam FIELDS       fields to which grant access
 */
template <typename BASE, template <typename> class... FIELDS>
struct SkinBase : FIELDS<BASE>..., BASE
                  // order above important for empty base class optimisation
{
    // make sure that no user puts data in a field...
    static_assert(SOAUtils::ALL<std::is_empty, FIELDS<BASE>...>::value,
            "Fields may not contain data or virtual methods!");
    /// inform the framework that we're a skin
    using skin_tag = struct {};
    /// we're the base class underlying classes that inherit from us
    using base_type = SkinBase;
    /// typelist listing all fields
    using fields_typelist = SOATypelist::typelist<FIELDS<void>...>;
    /// forward to *underlying constructors operators
    using BASE::BASE;
    /// forward to *underlying assignment operators
    using BASE::operator=;
    // FIXME: extract fields -> new skin class, new field tags
    // FIXME: make skin subobject -> new skin class, new field tags
};

/// little helper to detect the base type of a skin (saves typing)
template <typename T>
using detect_skin_base = typename T::base_type;

/// defined a field with name name and type type, with custom accessors
#define SOAFIELD_CUSTOM(name, type) \
    template <typename SKIN = void> \
    struct name : FieldBase<type, name, SKIN>
/// define a field with name name, accessors named accessorname, of type type
#define SOAFIELD(name, accessorname, type) \
    SOAFIELD_CUSTOM(name, type) { \
        type& accessorname() { return this->_get(); } \
        const type& accessorname() const { return this->_get(); } \
    }
/* user code */
SOAFIELD(f_x, x, float);
SOAFIELD(f_y, y, float);
SOAFIELD_CUSTOM(f_flags, int) {
    enum Flag { Used = 0x1, Dead = 0x2 };
    int& flags() { return this->_get(); }
    const int& flags() const { return this->_get(); }
    bool isUsed() const { return flags() & Used; }
    bool isDead() const { return flags() & Dead; }
    bool setUsed(bool newState = true)
    {
        int retVal = flags();
        flags() = (retVal & ~Used) | (-newState & Used);
        return retVal & Used;
    }
    bool setDead(bool newState = true)
    {
        int retVal = flags();
        flags() = (retVal & ~Dead) | (-newState & Dead);
        return retVal & Dead;
    }
};

template <typename BASE>
struct Skin : SkinBase<PrintableNullSkin<BASE>, f_x, f_y, f_flags>
{
    using B = detect_skin_base<Skin>;
    using B::B;
    using B::operator=;
    // special constructors go here...
    // special accessors go here...
};

/* small test driver... */
int main()
{
    SOAContainer<std::vector, NullSkin, float, float, int> s;
    SOAContainer<std::vector, Skin> c;
    static_assert(sizeof(s.front()) == sizeof(c.front()),
            "Fancy field and old-style field proxies need to have same size.");
    c.push_back(std::make_tuple(3.14f, 2.79f, 42));
    // print c[0]
    std::cout << c[0] << std::endl;
    // work a bit with c[0]
    std::cout << c[0].x() << " " << c[0].y() << " " << c[0].isUsed() << std::endl;
    c.front().setDead(false);
    // this won't work - constness
    // const_cast<const decltype(c)&>(c).front().setUsed();
    //
    // ... and print it again
    std::cout << c[0] << std::endl;
    struct Foo { int i; };
    SkinBase<Foo, f_x, f_y, f_flags> sb;
    static_assert(sizeof(int) == sizeof(sb), "skin size behaviour is all wrong.");
    return 0;
}

// vim: sw=4:tw=78:ft=cpp:et
