### Navigation: [<< (previous)](intro-1.4.md), [(up)](tutorial.md), [(next) >>](viewzip-2.2.md)

![SOA Container logo](../doc/SOAContainer.svg)
# Views
You have worked with ```SOA::Container<...>``` before in the introduction.
They are convenient when you build an entire new SOA object from scratch.
However, in many cases, one would like to construct objects "in stages", or
put together fields from two or more corresponding objects to form a new
object. This can be achieved with a ```SOA::View```. A ```SOA::View```
basically behaves like a fixed length range of SOA objects, so it's much
like a ```SOA::Container``` without the ability to insert or erase objects,
or change the length of the underlying range. In fact, every
```SOA:::Container``` inherits from ```SOA::View```, and adds functionality
like ```insert```, ```push_back```, ```clear```, and so on.

## Views as a selection of fields
Views can be used to select a subset of fields from a container. This can be
accomplished with code like this:

```c++
namespace XYZPoint {
	SOAFIELD_TRIVIAL(x, x, float);
	SOAFIELD_TRIVIAL(y, y, float);
	SOAFIELD_TRIVIAL(z, z, float);
	SOASKIN_TRIVIAL(Skin, x, y, z);
};
SOA::Container<std::vector, Point::Skin> c = /* fill somehow... */;
// we now have a big vector of points in 3D, cartesian coordinates...
// get the x/y portion of it
//
// in principle one could write an explicit type here, but SOA::View needs
// an awful amount of template arguments, so it's recommended to use the
// auto keyword from C++11 here
auto xyview = c.view<XYZPoint::x, XYZPoint::y>();
// one can also restrict the view to only a few elements:
// auto xyview = c.view<XYZPoint::x, XYZPoint::y>(c.begin(), c.size() / 2);

// let's see how we can convert them to cylindrical coordinates in a nice
// manner
namespace RPhiPoint {
	SOAFIELD_TRIVIAL(r, r, float);
	SOAFIELD_TRIVIAL(phi, phi, float);
	SOASKIN_TRIVIAL(Skin, r, phi);
};
SOAContainer<std::vector, RPhiPoint::Skin> crphi;
crphi.reserve(xyview.size());
for (auto xy: xyview) {
	crphi.emplace_back(
		std::sqrt(xy.x() * xy.x() + xy.y() * xy.y()),
		std::atan2(xy.y(), xy.x()));
}
```

As you can see, a ```SOA::View``` can be used much like any other range. It
is important at this point to realise that creating a new view does not copy
the data for the fields in question. Instead, it copies a bunch of iterators
for each field in the view which indicate the first and last elements in the
original container. That makes creating a view a cheap operation.

A word is in order concerning the *skin* of the view: If nothing else is
specified, the ```view<fields...>()``` and ```view<fields...>``` methods
will create a trivial skin from the fields listed as template arguments.
This is what most people want, and should not surprise too much. If you want
some non-trivial skin, you have to supply it as first template argument, as
in ```view<skin, fields...>```.

In the next section, we'll zip views together to form new views, in our
case, a point in 3D given in cylindrical coordinates.

### Navigation: [<< (previous)](intro-1.4.md), [(up)](tutorial.md), [(next) >>](viewzip-2.2.md)