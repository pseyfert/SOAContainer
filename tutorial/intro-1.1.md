### Navigation: [<< (previous)](tutorial.md), [(up)](tutorial.md), [(next) >>](intro-1.2.md)

![SOA Container logo](../doc/SOAContainer.svg)
# The simple AOS case
To illustrate how to turn an AOS object into SOA form, I will start with a
very simple example here (something more realistic is in the test code in
git). Consider a minimal point with x and y coordinates:

```cpp
namespace AOSPoint {
    class Point {
        private:
            float m_x;
            float m_y;
        public:
            Point(float x, float y) : m_x(x), m_y(y) { }
            float x() const noexcept { return m_x; }
            float y() const noexcept { return m_y; }
    	      float& x() noexcept { return m_x; }
  	        float& y() noexcept { return m_y; }
           // plus some routines that do more than just setting/getting members
            float r2() const noexcept { return m_x * m_x + m_y * m_y; }
    };
}

// define types for the AOS container and its contents
using AOSPoints = std::vector<AOSPoint::Point>;
using AOSPoint = typename AOSPoints::reference;
```

Then we all know how to use `AOSPoints` and `AOSPoint`. For example:

```cpp
AOSPoints points = /* get from somewhere */;

// normalise to unit length
for (AOSPoint p: points) {
    auto ir = 1 / std::sqrt(p.r2());
    p.x() *= ir, p.y() *= ir;
}
```

So far, so good.

### Navigation: [<< (previous)](tutorial.md), [(up)](tutorial.md), [(next) >>](intro-1.2.md)