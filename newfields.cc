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
 */
template <typename T, class FIELD>
class FieldBase {
    public:
        /// the field tag itself
        using self_type = FIELD;
        /// tell SOAContainer/View that we wrap a type
        using wrap_tag = struct {};
        // make sure that the user doesn't do strange things...
        static_assert(!SOATypelist::is_wrapped<T>::value,
                "Error: Do no nest FieldBase<FieldBase<T>, FIELD>!");
        /// type we wrap
        using type = T;
    protected:
        /** @brief low-level struct for as base class for accessors
         *
         * @tparam SKIN skin which this accessor will use
         */
        template <class SKIN>
        struct AccessorBase {
            /// retrieve reference to the field
            type& _get()
            { return reinterpret_cast<SKIN&>(*this).template get<FIELD>(); }
            /// retrieve const reference to the field
            const type& _get() const
            { return reinterpret_cast<const SKIN&>(*this).template get<FIELD>(); }
        };
};

/** @brief base class of all skins
 *
 * @tparam BASE         underlying tuple/whatever
 * @tparam FIELDS       fields to which grant access
 */
template <typename BASE, class... FIELDS>
struct SkinBase : FIELDS::template accessors<BASE>..., BASE
                  // order above important for empty base class optimisation
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
    // FIXME: extract fields -> new skin class, new field tags
    // FIXME: make skin subobject -> new skin class, new field tags
};

/// little helper to detect the base type of a skin (saves typing)
template <typename T>
using detect_skin_base = typename T::base_type;

/// defined a field with name name and type type, with custom accessors
#define SOAFIELD_CUSTOM(name, type, ... /* body */) \
    struct name : FieldBase<type, name> { \
        template <class SKIN> struct accessors : AccessorBase<SKIN> \
        { __VA_ARGS__ }; }
/// define a field with name name, accessors named accessorname, of type type
#define SOAFIELD(name, accessorname, type) \
    SOAFIELD_CUSTOM(name, type, \
        type& accessorname() { return this->_get(); } \
        const type& accessorname() const { return this->_get(); } \
    )
/// define a skin
#define SOASKIN_DECL(name, ... /* fields */) \
template <class BASE> struct name : SkinBase<PrintableNullSkin<BASE>, __VA_ARGS__>
#define SOASKIN_INHERIT_CONSTRUCTORS_ASSIGNMENTS(name) \
    using B = detect_skin_base<name>; \
    using B::B; using B::operator=

/* user code */
SOAFIELD(f_x, x, float);
SOAFIELD(f_y, y, float);
SOAFIELD_CUSTOM(f_flags, int,
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
    void printflags() { std::printf("flags: %08x\n", flags()); }
);

SOASKIN_DECL(Skin, f_x, f_y, f_flags) {
    SOASKIN_INHERIT_CONSTRUCTORS_ASSIGNMENTS(Skin);
    // special constructors go here...
    // special accessors go here...
    void setDeadIfTooFarOut()
    {
        // inside the skin, this->accessor() is required to make C++ find the
        // routine, users of the skin can just call accessor()
        auto x = this->x(), y = this->y();
        if ((x * x + y + y) > 1.f) this->setDead();
    }
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
