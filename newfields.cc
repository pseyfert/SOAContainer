#include <tuple>
#include <vector>
#include <iostream>
#include "include/SOAContainer.h"
#include "include/SOAField.h"
#include "include/SOASkin.h"

/* user code */
SOAFIELD(x, float);
SOAFIELD(y, float);
SOAFIELD_CUSTOM(f_flags, int,
    SOAFIELD_ACCESSORS(f_flags, int, flags)
    enum Flag { Used = 0x1, Dead = 0x2 };
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

SOASKIN(SkinSimple, f_x, f_y, f_flags);
SOASKIN_CUSTOM(Skin, f_x, f_y, f_flags) {
    SOASKIN_DEFAULT_CONSTRUCTORS_AND_ASSIGNMENTOPERATORS(Skin);
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
    SOASkin_impl::SkinBase<Foo, f_x, f_y, f_flags> sb;
    static_assert(sizeof(int) == sizeof(sb), "skin size behaviour is all wrong.");
    return 0;
}

// vim: sw=4:tw=78:ft=cpp:et
