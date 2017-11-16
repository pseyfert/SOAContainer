### Navigation: [<< (previous)](intro-1.3.md), [(up)](tutorial.md), [(next) >>](viewzip-2.1.md)

![SOA Container logo](../doc/SOAContainer.svg)
# Non-trivial fields, fields of type `bool`
The main thing missing from this basic tour is a demonstration of
non-trivial fields. As an example, let's work out a way to work around the
unsuitable `std::vector<bool>` specialisation (which packs 32 bits into a
word in vector with integers) by building a field for flags:

```cpp
namespace PointFields {
    // some fields...

    // define a flags field
    SOAFIELD(flags, unsigned,
        // standard getter and setter
        SOAFIELD_ACCESSORS(flags)

        // enum with flag names
        enum { Used = 0x1, Hot = 0x2, Dead = 0x4 };

        // some getters for individual flags
        bool used() const noexcept { return flags() & Used; }
        bool hot() const noexcept { return flags() & Hot; }
        bool dead() const noexcept { return flags() & Dead; }

        // some setters for individual flags
        void setUsed(bool used = true) noexcept
        { flags() = (flags() & ~Used) | (-used & Used); }
        void setHot(bool hot = true) noexcept
        { flags() = (flags() & ~Hot) | (-hot & Hot); }
        void setDead(bool dead = true) noexcept
        { flags() = (flags() & ~Dead) | (-dead & Dead); }
    );
}
```

This defines a field `PointFields::flags`, which holds an `unsigned`. It
has a getter/setter `flags()` to get the underlying integer, an `enum` with
three defined flags `Used`, `Hot`, and `Dead`. There are three getters for
these flags, `used()`, `hot()`, and `dead()`, and three setters `setUsed()`,
`setHot()` and `setDead()`. With this, we can define a point which has
flags, too:

```cpp
// a more complicated skin offering methods beyond what fields provide
SOASKIN(SOAPointSkin, PointFields::x, PointFields::y, PointFields::flags) {
    // fall back on defaults...
    SOASKIN_INHERIT_DEFAULT_METHODS;
    // your own constructors etc. go here (if you have any)...

    // we inherit getters/setters from fields

    // again, something beyond plain setters/getters
    float r2() const noexcept
    { return this->x() * this->x() + this->y() * this->y(); }
    // set dead flag of a point, if too far away from origin
    void setDeadIfTooFarOut() noexcept
    { setDead(r2() > 2500.f); }
};
```

### Navigation: [<< (previous)](intro-1.3.md), [(up)](tutorial.md), [(next) >>](viewzip-2.1.md)