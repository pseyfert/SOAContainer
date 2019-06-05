### Navigation: [<< (previous)](intro-1.4.md), [(up)](tutorial.md), [(next) >>](viewzip-2.1.md)

![SOA Container logo](../doc/SOAContainer.svg)
# Tagged types in constructors and `emplace_back`

Imagine you have a shiny nice SOA 3D point, and you want to append points to the end of a container:

``` c++
#include "SOATaggedType.h"
using namespace SOA;
SOAFIELD_TRIVIAL(f_x, x, float);
SOAFIELD_TRIVIAL(f_y, y, float);
SOAFIELD_TRIVIAL(f_z, z, float);
SOASKIN_TRIVIAL(Point3DSkin, f_x, f_y, f_z);
using Cont = Container<std::vector, Point3DSkin>
Cont c;
c.emplace_back(1.f, 2.f, 3.f);
// meant to construct the same point as c.back, but swapped order of x and y!
Cont::value_type point(2.f, 1.f, 3.f);
```

This code will work, but is a bit nasty, since you have to get the order of x,
y, z components right. (It's easy to do by convention in this case, but even
easier to get wrong in more complex examples). It would therefore be nice if
the `SOA` framework had a mechanism to deal with this. Consider the following
improved code:

``` c++
// as before
using Cont = Container<std::vector, Point3DSkin>
Cont c;
c.emplace_back(value<f_x>(1.f), value<f_y>(2.f), value<f_z>(3.f));
// same as c.back(), despite the different order
Cont::value_type point(value<f_y>(2.f), value<f_x>(1.f), value<f_z>(3.f));
```

This code is not only much clearer, it also allows the `SOA` framework to
correct the order if you swap the x and y fields, for example.

These *tagged types* allow for quite a bit of flexibility in construction,
since they introduce a way of naming constructor arguments. In addition to
`value<field>`, you also have `ref<field>` which models a non-const reference
to a data item of the field type, and `cref<field>` which models a const
reference. These can be used in much the same way as you would use a `T`, `T&`
or `const T&` if your field refers to a `T`.

### Navigation: [<< (previous)](intro-1.4.md), [(up)](tutorial.md), [(next) >>](viewzip-2.1.md)
