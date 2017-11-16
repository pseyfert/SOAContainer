### Navigation: [<< (previous)](intro-1.2.md), [(up)](tutorial.md), [(next) >>](intro-1.4.md)

![SOA Container logo](../doc/SOAContainer.svg)
# Non-trivial SOA skins
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

### Navigation: [<< (previous)](intro-1.2.md), [(up)](tutorial.md), [(next) >>](intro-1.4.md)