### Navigation: [<< (previous)](faq-4.md), [(up)](tutorial.md), [(next) >>](internals-5.2.md)

![SOA Container logo](../doc/SOAContainer.svg)
# 5.1 Tuples and Typelists

The original idea for SOA containers was that each container is basically
just a tuple of vectors. So if you'd like to save, say, an ```int```, a
```float``` and a ```double```, your basic data structure would look
something like this:

```C++
struct MySOAContainer {
	std::tuple<std::vector<int>,
		std::vector<float>,
		std::vector<double> > m_data;
};
```

It's obvious that the struct has a terrible user interface, as the user
has the full responsibility to keep the three vectors in the tuple in sync.
Obviously, the ```SOA::Container``` class has gotten virtually the same
interface that ```std::vector``` uses. People are familiar with it, and
it helps keeping the internal vectors in the tuple in sync.

The first interesting bit is therefore how the container class finds out
which tuple element goes with the three different types, ```int```,
```float```, and ```double```.

Enter the *type list*: It's a compile-time list of types. (If you're
interested in the implementation, you can peek at ```SOATypelist.h```)

```
struct MySOAContainer {
	// ... many data members and methods ...
	using fields_typelist = SOA::Typelist::typelist<int, float, double>;
	// ... many data members and methods ...
};
```

In this typelist, one can have the compiler look for the position where a
type occurs in the list: ```fields_typelist.find<int>()``` will (at compile
time) return 0, ```fields_typelist.find<float>()``` will return 1, and so
on. Thus, there is a way to associate the elements in the tuple with the
three different fields in our SOA container.

What's more, one can package that such that a SOA container can be indexed,
and returns something that behaves essentially like a tuple of the types in
the typelist. The first crucial idea here is that indexing a container
and deferencing an iterator return a proxy that stands in for the
```tuple<int, float, double>``` corresponding to the relevant elements in
the vectors in ```m_data``` above (look at ```SOAObjectProxy.h```). The
second important idea is that such a tuple-like object needs to be *dressed*
a bit to allow a more convenient way to access the fields (look at
```SOADressedTuple.h```):

```C++
SOA::Container<...> c = /* ... */;
c.front() = std::make_tuple(42, 3.14f, 2.79); // int, float, double
int myint = c.front().get<int>();		// 42
float myfloat = c.front().get<float>();	// 3.14f
```

That's already almost what one wants. The next sections discusses the
limitations of that approach that led to fields and skins.

### Navigation: [<< (previous)](faq-4.md), [(up)](tutorial.md), [(next) >>](internals-5.2.md)