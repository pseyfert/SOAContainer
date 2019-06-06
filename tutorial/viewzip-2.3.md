### Navigation: [<< (previous)](viewzip-2.2.md), [(up)](tutorial.md), [(next) >>](future-3.md)

![SOA Container logo](../doc/SOAContainer.svg)
# Algorithms

To make processing with SOA Containers more convenient, the framework defines a
couple of algorithms which take a view and a function or functor to drive the
processing. The tagged types from section [1.5](intro-1.5.md) are used to limit
the processing to the fields that are required by the functor.

## `for_each`

`for_each` is the simplest algorithm. It just calls a function or functor for
each element in the view. The fields that are interesting for the processing
are selected with the argument list of the functor or function:

``` c++
#include "SOAAlgorithm.h"
using namespace SOA;
auto view = /* from somewhere */;
// assume field_x and field_y are floats, and field_flags is a flag field that
// has a method called setInsideBeamPipe

// determine which points are inside the beam pipe, and set a flag
for_each(view, [] (cref<field_x> x, cref<field_y> y, ref<field_flags> flags) {
	if (x * x + y * y < 25) flags.setInsideBeamPipe();
    });
```

As you can see, the tagged types (`value<field>`, `ref<field>`, `cref<field>`)
behave as the underlying data type (field_x and field_y behave as  values or
references of floats that they represent. `ref<field_flags>` behaves as a
reference to the flag data type would (i.e. it inherits the field's methods and
accessors).

Moreover, if the data type of an argument identifies the field to be taken from
the view uniquely and umambiguously, you can even omit the tagged data type:

``` c++
auto view = /* from somewhere */;
// assume view contains a single LHCb::State, and the flag field from the last
// code fragment

// flag elements which have a state inside the beam pipe
for_each(view, [] (const LHCb::State& state, ref<field_flags> flags) {
	if (st.x() * st.x() + st.y() * st.y() < 25) flags.setInsideBeamPipe();
    });
```

If there are two fields which have type LHCb::State, the code will print an
error message, and refuse to compile. The tagged type mechanism from above will
work to break the ambiguity in such a case.

### Navigation: [<< (previous)](viewzip-2.2.md), [(up)](tutorial.md), [(next) >>](future-3.md)
