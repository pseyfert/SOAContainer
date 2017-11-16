### Navigation: [<< (previous)](future-3.md), [(up)](tutorial.md), [(next) >>](internals-5.1.md)

![SOA Container logo](../doc/SOAContainer.svg)
# Frequently Asked Questions (FAQ)

## 1. Why does this ```SOA``` stuff crash (or fail to compile) in my first range-based ```for``` loop?

   Well, this has to do with the fact that an SOA container does not contain
   the objects themselves in AOS memory layout, but is a collection of
   fields. Thus, using ```operator[]``` or deferencing an iterator into the
   container cannot return a reference to the object - the object itself
   doesn't even exist in memory!

   Instead, it will return a *proxy* object which stands in for the object,
   and will behave for most intents and purposes as if the object itself
   was returned by reference. For example, the declaration of the
   deferencing operator in an iterator could be written like this:

   ```C++
   proxy_object operator*();
   ```

   Now let's look what happens in the typical range-based for loop that a
   well-trained user will write:

   ```C++
   SOAContainer<...> container = /* ... */;
   for (auto& el: container) {
	   el.foo(el.x());
   }
   ```

   For the range-based for loop, the C++ compiler will produce code
   equivalent to this:

   ```C++
   {
	   const auto end = container.end();
	   for (auto it = container.begin(); end != it; ++it) {
		   auto& el = *it; // TROUBLE!!!
		   el.foo(el.x());
	   }
   }
   ```

   Things go wrong in the line marked with the comment ```// TROUBLE!!!```.
   As we just said, ```*it``` returns a proxy object. We do not save that
   proxy anywhere, we just take a reference to it. Since the proxy itself
   isn't used, its destructor is called. And after that, we start following
   the reference to a proxy object that doesn't even exist any more.

   The solution here is to realise that proxy objects already have reference
   semantics: They are relatively lightweight (basically the size of a
   pointer-index pair), and act like a reference because they modify the
   state of the underlying container. Therefore, it's perfectly okay to
   write the following code:

   ```C++
   for (auto el: container) {
	   el.foo(el.x());
   }
   ```

   This saves a copy of the proxy object. The code will perform well, it's
   less typing, and, best of all, it will just work, and do what you want
   and expect.


## 2. How do I get ```std::sort``` to work?

  ```std::sort``` is a bit tricky when used with ```SOA::Container```.
  Typically, you have a piece of code like this:

```C++
namespace SOAHit {
	SOAFIELD_TRIVIAL(x0, x0, float);
	SOAFIELD_TRIVIAL(z0, z0, float);
	SOAFIELD_TRIVIAL(dxdy, dxdy, float);
	/// ...
	SOASKIN_TRIVIAL(Skin, x0, z0, dxdy /* , ... */ );
}
using Hits = SOA::Container<std::vector, SOAHit::Skin>;

Hits c = /* ... */;
std::sort(c.begin(), c.end(), [] (Hits::const_reference a, Hits::const_reference b)
	{ return a.x0() < b.x0(); });
```

This will not work because ```std::sort``` saves objects outside the
container while performing its work. Specifically, most ```std::sort```
implementations contain code like this:

```C++
// it and jt are iterators into a range; it should be moved to the the right
// position inside the range while preserving the order of the other
// elements; cmp is the comparison functor
//
// the following code finds the right place to insert *it
//
// in the SOA::Container case, *it is a proxy object, and value_type is
// a (potentially dressed and skinned) tuple of raw fields, i.e. it's an
// AOS-style object
value_type save(std::move(*it)); // save element outside of container
// move other elements until we reach place where save needs inserting
for ( ; jt != it + 1 && cmp(*it + 1, save); ++it)
	*it = std::move(*(it + 1));
*it = std::move(save); // put element into container in its final position
```

As you can see, the comparison functor gets called with different types of
arguments, and not necessarily the ones you expect.

There's two ways around this. The first one, available in C++14 or better
is the easiest:

```C++
namespace SOAHit {
	SOAFIELD_TRIVIAL(x0, x0, float);
	SOAFIELD_TRIVIAL(z0, z0, float);
	SOAFIELD_TRIVIAL(dxdy, dxdy, float);
	/// ...
	SOASKIN_TRIVIAL(Skin, x0, z0, dxdy /* , ... */ );
}
using Hits = SOA::Container<std::vector, SOAHit::Skin>;

Hits c = /* ... */;
std::sort(c.begin(), c.end(), [] (const auto& a, const auto& b)
	{ return a.x0() < b.x0(); });
```

If your compiler does not support c++14 yet, you can use this workaround:

```C++
namespace SOAHit {
	SOAFIELD_TRIVIAL(x0, x0, float);
	SOAFIELD_TRIVIAL(z0, z0, float);
	SOAFIELD_TRIVIAL(dxdy, dxdy, float);
	/// ...
	SOASKIN_TRIVIAL(Skin, x0, z0, dxdy /* , ... */ );
}
using Hits = SOA::Container<std::vector, SOAHit::Skin>;

Hits c = /* ... */;
std::sort(c.begin(), c.end(), [] (Hits::value_const_reference a,
	Hits::value_const_reference b) { return a.x0() < b.x0(); });
```

The option above is slightly more typing, but will work with any C++11
compiler.

### Navigation: [<< (previous)](future-3.md), [(up)](tutorial.md), [(next) >>](internals-5.1.md)