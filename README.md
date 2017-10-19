# SOAContainer

SOAContainer is a class that mimics the interface of std::vector as much
as possible. In fact, from a user's point of view, there is not too much
difference between a `SOA::Container` and an array of structure (AOS)
`std::vector<Hit>` where Hit is an old-style class/structure that we all
know. You can sort, push_back, emplace, insert, erase, reserve, size,
index and get iterators from a `SOA::Container` just like you would for the
AOS case. Moreover, there's the `SOA::View` class which does the same
forward fixed-size ranges.

You can read the details in the [tutorial](./tutorial/tutorial.md).

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

