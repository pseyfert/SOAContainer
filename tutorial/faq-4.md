### Navigation: [<< (previous)](future-3.md), [(up)](tutorial.md), [(next) >>](tutorial.md)

# Frequently Asked Questions (FAQ)

1. Why does this ```SOA``` stuff crash (or fail to compile) in my first
   range-based ```for``` loop?

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

### Navigation: [<< (previous)](future-3.md), [(up)](tutorial.md), [(next) >>](tutorial.md)