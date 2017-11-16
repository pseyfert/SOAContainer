### Navigation: [<< (previous)](intro-1.1.md), [(up)](tutorial.md), [(next) >>](intro-1.3.md)

![SOA Container logo](../doc/SOAContainer.svg)
# The trivial SOA case
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

### Navigation: [<< (previous)](intro-1.1.md), [(up)](tutorial.md), [(next) >>](intro-1.3.md)
