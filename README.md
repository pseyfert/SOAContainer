# SOAContainer

SOAContainer is a class that mimics the interface of std::vector as much
as possible. In fact, from a user's point of view, there is not too much
difference between a `SOA::Container` and an array of structure (AOS)
`std::vector<Hit>` where Hit is an old-style class/structure that we all
know. You can sort, push_back, emplace, insert, erase, reserve, size,
index and get iterators from a `SOA::Container` just like you would for the
AOS case. Moreover, there's the `SOA::View` class which does the same
forward fixed-size ranges.

## A very short tutorial
### The simple AOS case
To illustrate how to turn an AOS object into SOA form, I will start with a
very simple example here (something more realistic is in the test code in
git). Consider a minimal point with x and y coordinates:

```cpp
namespace AOSPoint {
    class Point {
        private:
            float m_x;
            float m_y;
        public:
            Point(float x, float y) : m_x(x), m_y(y) { }
            float x() const noexcept { return m_x; }
            float y() const noexcept { return m_y; }
    	      float& x() noexcept { return m_x; }
  	        float& y() noexcept { return m_y; }
           // plus some routines that do more than just setting/getting members
            float r2() const noexcept { return m_x * m_x + m_y * m_y; }
    };
}

// define types for the AOS container and its contents
using AOSPoints = std::vector<AOSPoint::Point>;
using AOSPoint = typename AOSPoints::reference;
```

Then we all know how to use `AOSPoints` and `AOSPoint`. For example:

```cpp
AOSPoints points = /* get from somewhere */;

// normalise to unit length
for (AOSPoint p: points) {
    auto ir = 1 / std::sqrt(p.r2());
    p.x() *= ir, p.y() *= ir;
}
```

So far, so good.

### The trivial SOA case
Unfortunately, the memory layout is AOS, i.e. not what SIMD units in modern
CPUs like. Converting this to a SOA layout is fairly easy, though. Let's
start with the fields (data members):

```cpp
#include "SOAContainer.h"

// first declare fields which describe the members of the notional struct
// (which will never exist in memory - SOA layout!)
 namespace PointFields {
    SOAFIELDS_TRIVIAL(x, x, float); // field struct x, getter/setter x(), type float
    SOAFIELDS_TRIVIAL(y, y, float);
};
```

This code above defines two structures in the namespace `PointFields` using
the `SOAFIELDS_TRIVIAL` macro. Their names are `x` and `y` (first argument),
which are used to identify the fields. These structures provide getters and
setters, called `x()` or `y()` (second argument), returning (const)
references to the underlying `float` (data type is third argument).
It's called a trivial field because it just provides standard getters and
setters.

The next step is to form skin, which is a kind of "decorator class" which
embellishes a tuple of data members with the interface we want it to have:

```cpp
// define the "skin", i.e. the outer guise that the naked members "wear"
// to make interaction with the class nice
SOASKIN_TRIVIAL(SOAPointSkinSimple, PointFields::x, PointFields::y);
```

The macro `SOASKIN_TRIVIAL` takes a name of the skin as first argument, and
a list of fields, and produces a skin that has all the user interface
provided by the fields. We're now ready to use our SOA container:

```cpp
// define the SOA container type
using SOASimplePoints = SOA::Container<
        std::vector,	        // underlying type for each field
        SOAPointSkinSimple>;  // skin to "dress" the tuple of fields with
// define the SOASimplePoint itself
using SOASimplePoint = typename SOASimplePoints::reference;

// make a container of simple points
SOASimplePoints points = /* get from somewhere... */;

// normalise to unit length
for (SOAPoint p: points) {
    auto ir = 1 / std::sqrt(p.x() * p.x() + p.y() * p.y());
    p.x() *= ir, p.y() *= ir;
}
```

At this point, you have a container that behaves basically like the
std::vector in the AOS case, and you should feel right at home when writing
code.

### Non-trivial SOA skins
The problem with trivial fields and skins in the code example above is that
they are limited to the trivial getters and setters provided by the fields.
It would be nice if we could somehow "connect" the data of different fields
in a skin to provide a method that provides e.g. the squared distance of a
point from the origin. Fortunately, this is not much more difficult:

```cpp
// a more complicated skin offering methods beyond what fields provide
SOASKIN(SOAPointSkin, PointFields::x, PointFields::y) {
    // fall back on defaults...
    SOASKIN_INHERIT_DEFAULT_METHODS;
    // your own constructors etc. go here (if you have any)...

    // we inherit getters/setters from fields

    // again, something beyond plain setters/getters
    float r2() const noexcept
    { return this->x() * this->x() + this->y() * this->y(); }
};
```

The "feel" of the `SOASKIN` macro should by now be familiar. Next, the
`SOASKIN_INHERIT_DEFAULT_METHODS` macro is invoked to inherit constructors
and assignment operators from the underlying tuples. You can define your
own ones, too, as indicated in the comment. Finally, we move on to whatever
custom methods are needed, in our case `r2()`.

From that point on, you can more or less do what you're used to with your
points, the SOA nature should largely be invisible. For example, the code
normalising the lengths of points would require virtually no changes when
going from the AOS point to the SOA point class:

```cpp
// define the SOA container type
using SOAPoints = SOA::Container<
        std::vector,	        // underlying type for each field
        SOAPointSkin>;  // skin to "dress" the tuple of fields with
// define the SOASimplePoint itself
using SOAPoint = typename SOAPoints::reference;

// make a container of simple points
SOAPoints points = /* get from somewhere... */;

// normalise to unit length
for (SOAPoint p: points) {
    auto ir = 1 / std::sqrt(p.r2());
    p.x() *= ir, p.y() *= ir;
}
```

The fields (members) in a SOAContainer-contained object can essentially be of
any POD type or pointer type, and there can be as many as the compiler will
support - if you need something more specialised, it'll likely work, but may
need some tweaking... Be careful with `bool` fields, though, because
`std::vector<bool>` is specialised, and will most likely spoil your SOA
performance (see below for a workaround...).

### Non-trivial fields, fields of type `bool`
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

# TO BE REVIEWED


A few notes at this point seem to be in order:

- The container is internally a tuple<std::vector<field_1_type>,
  std::vector<field_2_type>, ...>. Other containers that conform to
  std::vector's interface can be used. Wouter's StaticArray from DetDesc
  comes to mind (essentially a std::array plus a size with std::vector
  semantics) - with a little bit of work to bring it to comply with
  std::vector's interface, we can make that happen...

- The NAKEDPROXY object is internally a pair of pointer to SOAContainer
  instance and an index into it, and it supports assignment
  from/conversion to tuple<field_1_type, field_2_type, ...> and
  tuple<field_1_type&, field_2_type&, ...> (for std::tie to work).

  It also supports the get<0>(), get<1>(), ... syntax to retrieve the
  field for the first, second, ... field template argument to
  SOAContainer, and it supports the more friendly symbolic lookup with
  get<FieldTag>() I've used in the example above.

  You can take the "address" of such a proxy object and get a
  pointer/iterator, which, when deferenced/indexed yields back another
  proxy.

- Since NAKEDPROXY itself isn't too friendly on the user, all the proxies
  are "skinned" with something more pleasant, in our case, the SOAPoint
  skin class. If you prefer the "naked" or skinless appearance, there's a
  NullSkin class in SOAContainer.h.

In the test code in the repo, there's an example which essentially takes
the example of a Tf::LineHit, and implements the corresponding SOA class.

According to callgrind, gcc 4.9 with appropriate flags will autovectorise
a gain of around 25% over the AOS version when shifting hits along y
(based on a mock-up of updateNonOTHitForTrack in Tf/RecoFuncs.h).
Unfortunately, in clang 3.5, the SOA version is around 10% slower than
the AOS version - to be understood.

I've had a brief look at the assembly in both cases, and both compilers
struggle, spilling registers on the stack. Still, given the register
shortage, both autovectorise reasonably well. Moreover, I would expect
things to improve once we have StaticArray working because then the
starting addresses are not random (from new), but will have a fixed
offset from one member/field container to the next, and the compiler
could take advantage of that by not dragging around five-six pointers,
and using one pointer, and five-six compile-time known offsets... But
we'll have to see...

SOAView:
--------
This is much like SOAContainer, but is constructed from a set of
pre-existing ranges. The resulting container can be changed if the range is not
const, but its length is fixed. Again, the fields can be pretty much anything.
A simple example follows:

```cpp
// FIXME: new style example
#include <SOAView.h>
using field_x = struct : SOATypelist::wrap_type<float> {};
using field_y = struct : SOATypelist::wrap_type<float> {};
template <typename NAKEDPROXY>
class SOAPoint : public NAKEDPROXY {
    public:
        using fields_typelist = SOATypelist::typelist<field_x, field_y>;
        using NAKEDPROXY::NAKEDPROXY;
        using NAKEDPROXY::operator=;

        float x() const noexcept
        { return this-> template get<field_x>(); }
        float y() const noexcept
        { return this-> template get<field_y>(); }
        float& x() noexcept
        { return this-> template get<field_x>(); }
        float& y() noexcept
        { return this-> template get<field_y>(); }
        float r2() const noexcept { return x() * x() + y() * y(); }
};
std::vector<float> vx, vy;
// fill vx, vy somehow - same number of elements

// construct a SOAView from vx, vy
auto view = make_soaview<SOAPoint>(vx, vy);
// rotate points inside the unit circle by a given angle
const float angle = 42.f / 180.f * M_PI;
const auto s = std::sin(angle), c = std::cos(angle);
for (auto p: view) {
    if (p.r2() > 1) continue;
    // rotate points within the unit circle by given angle
    std::tie(p.x(), p.y()) = std::make_pair(
            c * p.x() + s * p.y(), -s * p.x() + c * p.y());
}
```

In this case, two vectors play the role of the ranges, but pretty much anything
with a range-like interface (i.e. a begin(), and end(), a size() and an empty())
will do. SOAViews allow the construction of objects in stages - one can start
by filling some vectors with the basic information, create a view, calculate
some more fields into another bunch of vectors, then create a new view that
takes the newly calculated fields into account, and so on.


Future plans:
-------------
- improve documentation (Doxygen)
- clean up the interface
- investigate if supporting the std::initializer_list interface of
  std::vector makes sense (I think at one point I convinced myself that
  it does not, but I'm no longer so sure... ;)

Random notes on current status:
-------------------------------
- the typelist functions and what's built around it (especially
  recursive_apply_tuple) should be reviewed by someone who's familiar
  with functional languages and template metaprogramming

- the iterator/pointer classes come in two guises:

  * a const iterator class which is the base
  * a non-const iterator class which derived from the const one

  this is done because a POD non-const pointer can always be converted to
  a const pointer without trouble, while converting the other way needs a
  cast. (copy/move) construction is the same: creating a const pointer
  from a non-const one is valid (implicitly down-casting from derived to
  base), but the other way is forbidden

  I've been having some difficulty with operator-> which is a little less
  than straight forward - it seems to work well enough as is, but I'd
  like to scrutinize this part to make sure there's no trouble lurking
  there before I release this bit into the wild

- the SOAObjectProxy and SOA(Const)Iterator classes should be reviewed to
  see if the noexcept specification makes sense everywhere; swap should
  be reviewed, and swap_ranges implemented

- not sure if all the copy/move constuctors/assignment operators we need
  are there, or if we have too many (to be seen)

- I'm sure there's more that needs improving and cleaning up, so don't be
  shy and tell me... ;)

