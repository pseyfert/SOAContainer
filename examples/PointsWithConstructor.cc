#include <iostream>
#include <array>
#include <vector>
#include <tuple>

#include "SOAContainer.h"
#include "PrintableNullSkin.h"

namespace AOS {
    class Point {
	private:
	    float m_x;
	    float m_y;
	public:
	    Point(float x, float y) : m_x(x), m_y(y) { }
      // some constructors which don't just list all members
      Point() : m_x(0.f), m_y(0.f) { }
      Point(float y) : m_x(1.f), m_y(y) { }
	    float x() const noexcept { return m_x; }
	    float y() const noexcept { return m_y; }
	    void setX(float x) noexcept { m_x = x; }
	    void setY(float y) noexcept { m_y = y; }
	    // plus some routines that do more than just setting/getting members
	    float r2() const noexcept { return m_x * m_x + m_y * m_y; }
    };

    std::ostream& operator<<(std::ostream& os, const Point& point) {
	return os << "{" << point.x() << ", " << point.y() << "}";
    }

    typedef std::vector<Point> Points;
    typedef Point& AOSPoint;
}

namespace SOA {
    namespace PointFields {
	using namespace SOA::Typelist;
	// since we can have more than one member of the same type in our
	// SOA object, we have to do some typedef gymnastics so the compiler
	// can tell them apart
	struct x : wrap_type<float> { };
	struct y : wrap_type<float> { };
    }

    template <typename NAKEDPROXY>
    class SOAPointProxy : public SOA::PrintableNullSkin<NAKEDPROXY> {
	public:
	    using PrintableNullSkin<NAKEDPROXY>::PrintableNullSkin;
	    using PrintableNullSkin<NAKEDPROXY>::operator=;
	    using fields_typelist =
		SOA::Typelist::typelist<PointFields::x, PointFields::y>;

      // some constructors which don't just list all members
      SOAPointProxy() : SOA::PrintableNullSkin<NAKEDPROXY>(
          0.f, // x
          0.f  // y
          ) {
        std::cout << "constructor with zero argument" << std::endl;
      }
      SOAPointProxy(float y) : SOA::PrintableNullSkin<NAKEDPROXY>(
          1.f, // x
          y
          ) {
        std::cout << "constructor with one argument" << std::endl;
      }
	    float x() const noexcept
	    { return this-> template get<PointFields::x>(); }
	    float y() const noexcept
	    { return this-> template get<PointFields::y>(); }
	    void setX(float x) noexcept
	    { this-> template get<PointFields::x>() = x; }
	    void setY(float y) noexcept
	    { this-> template get<PointFields::y>() = y; }

	    // again, something beyond plain setters/getters
	    float r2() const noexcept { return x() * x() + y() * y(); }
    };

    // define the SOA container type
    typedef SOA::Container<
	std::vector,   // underlying type for each field
	SOAPointProxy> // skin to "dress" the tuple of fields with
	Points;
    // define the SOAPoint itself
    typedef typename Points::proxy Point;
}

int main() {
    using namespace std;
    {
        using namespace AOS;
        cout << "This is a normal array of structures" << endl;
        Points list_of_points;
        list_of_points.emplace_back();     // should add 0,0
        list_of_points.emplace_back(2.f);  // should add 1,2
        list_of_points.emplace_back(4,5);  // should add 4,5

        for(const auto& item : list_of_points)
            cout << item << endl;

    }

    {
        using namespace SOA;

        cout << endl << "This is a SOA wrapper:" << endl;

        //Points list_of_points;
        Points list_of_points;
        list_of_points.emplace_back();     // should add 0,0
        list_of_points.emplace_back(2.f);  // should add 1,2
        list_of_points.emplace_back(4,5);  // should add 4,5

	// SOA containers return proxy classes, so don't use the reference in
	// the range-based for!
	for(auto item : list_of_points)
            cout << item << endl;


    }

    return 0;
}
