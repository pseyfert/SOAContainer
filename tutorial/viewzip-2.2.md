### Navigation: [<< (previous)](viewzip-2.1.md), [(up)](tutorial.md), [(next) >>](future-3.md)

# Zips

You remember the example from last section which took a view of the x and y
coordinates of a bunch of points in 3D in cartesian coordinates, and
calculates polar coordinates in the x-y plane:

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

Once you have that, it's trivial to combine the newly calculated r and phi
vectors with the z component of the original container, and produce a new
view of these three fields:

```C++
auto zview = c.view<XYZPoint::z>();
// be careful, here comes the magic that creates a view in cylindrical
// coordinates by zipping together existing views of the same length
auto view_cyl = zip(crphi, zview);
// you get back a normal view, and can do things with it:
for (auto p: view_cyl) {
    std::cout << "Point r " << p.r() << " phi " << p.phi() << " z " <<
        p.z() << std::endl;
}
```

The call to the ```zip``` method will check that all supplied views have
the same size (length of the underlying range), and that no fields appear in
duplicate, and if so, proceed to from a view by zipping together the ranges
for the underlying fields. The important point here is that the data itself
is not copied. Instead, for each field, a bunch of pointers indicating
beginning and end of the range is saved in the newly created view. This
makes the creation of views a cheap operation.

By default, the call to ```zip``` will give the resulting view a trivial
skin (i.e. just the "sum of all the fields"). If you would like to get
something more complex, you will need to specify the skin, as in
```zip<NewSkin>(views...)```.

This technique can be used to initialise a SOA object "in stages": One
starts with a ```SOA::Container```, or a ```SOA::View```. Then, the next
couple of fields are calculated from the data in the initial view (and
possibly other inputs), and is saved into a new container, while maintaining
a one-to-one correspondence between elements. Finally, the ```zip```
operation allows to zip together these separate bits to form a new joint
view.

### Navigation: [<< (previous)](viewzip-2.1.md), [(up)](tutorial.md), [(next) >>](future-3.md)