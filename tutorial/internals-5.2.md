### Navigation: [<< (previous)](internals-5.1.md), [(up)](tutorial.md), [(next) >>](tutorial.md)

# Old-style fields and skins

The simple typelist-based approach from the last section works, but it has
a couple of serious drawbacks if it's left as is:

- one can only have a single field of any given type, since searching the
  typelist cannot find out which of the several distinct fields of that
  type the user is talking about (e.g. in a typelist with two ```int```s,
  always returning the first one will most likely disappoint the user)
- it is kind of hard to remember which type saves which data item - a more
  descriptive way of naming would be desirable

## wrapping types: fields
To overcome these issues, types are *wrapped*:

```C++
template <typename T>
struct wrap_type {
    using type = T;             // the wrapped type
    using wrap_tag = struct {}; // so framework can recognise it's wrapped
};

// now we can have symbolic names for the fields, and also several fields
// of the same type:
namespace fields {
    using x = struct : public wrap_type<float> {};
    using y = struct : public wrap_type<float> {};
    using flags = struct : public wrap_type<int> {};
    using r = struct : public wrap_type<double> {};
}

using fields_typelist = SOA::Typelist::typelist<
    fields::x, fields::y, fields::flags, fields::r>;
```

Now, the fields have nice descriptive names, and we can even have more than
one that are of the same type. (It's still quite a mouthful to type, which
is why the more modern ```SOA_FIELD...``` macros were introduced...)

What is still missing is something that gives a more convenient user
interface. One would like to write ```element.x()``` and so on...

## Skins
This is where the idea of a *skin* comes in. You essentially have a class
that doesn't really care about how the data is saved internally, and just
adapts the user interface. With something that behaves like a dressed tuple
from the last section, one can write a simple skin:

```C++
template <typename T>
class Skin : public T {
    public:
        // reuse constructors and assignment operators from the underlying
        // representation
        using T::T;
        using T::operator=;

        // and provide a sane interface
        float x() const { return this->template get<fields::x>(); }
        float& x() { return this->template get<fields::x>(); }
        float y() const { return this->template get<fields::y>(); }
        float& y() { return this->template get<fields::y>(); }
        int flags() const { return this->template get<fields::flags>(); }
        int& flags() { return this->template get<fields::flags>(); }
        double r() const { return this->template get<fields::r>(); }
        double& r() { return this->template get<fields::r>(); }
};
```

This allows users to define essentially any user interface for their SOA
objects that they wish to have, and a SOA container will take care of the
details by taking or returning skinned tuples, SOA object proxies etc.

The trouble with this is that it's a huge amount of boilerplate code that
a prospective user needs to type. For that reason, a somewhat more flexible
approach to fields and skins was introduced. See the next sections for
details.

### Navigation: [<< (previous)](internals-5.1.md), [(up)](tutorial.md), [(next) >>](tutorial.md)