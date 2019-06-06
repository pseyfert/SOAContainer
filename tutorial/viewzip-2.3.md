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

## `transform`

`transform` is much like `for_each`, except that a new container is created
from the value(s) returned from the function of functor. The discussion about
the functor's argument list to select which fields are passed from `for_each`
above applies here as well.

The return type of the functor must be a tagged data type, or a tuple of tagged
data types. For the return type, tagged data types must be used since
`transform` uses it to extract the list of fields that the returned container
needs to have.

Here's a simple example:

``` c++
auto view = /* from somewhere */;

auto xztrackparam = transform(view, [] (cref<f_x1> x1, cref<f_z1> z1,
					cref<f_x2> x2, cref<f_z2> z2) {
	return std::make_tuple(value<f_x0>(0.5f * (x1 + x2),
			       value<f_z0>(0.5f * (z1 + z2)),
			       value<f_tx>((x2 - x1)/(z2 - z1)));
    });
// xztrackparam is a SOA container with three fields: f_x0, f_z0, f_tx
// which are set to the parameters of the line that goes through the
// input points (x1, z1) and (x2, z2)
```

If you just need to return a single field, you do not have to wrap it in a std::tuple:

``` c++
auto view = /* from somewhere */;
auto radii = transform(view, [] (cref<f_x> x, cref<f_y> y, cref<f_z> z) {
	return value<f_r>(std::sqrt(x * x + y * y + z * z));
    });
// radii is now a SOA container with a single field: f_r
```

### Navigation: [<< (previous)](viewzip-2.2.md), [(up)](tutorial.md), [(next) >>](future-3.md)
