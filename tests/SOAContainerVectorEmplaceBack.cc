/** @file tests/SOAContainerVectorEmplaceBack.cc
 *
 * @author Paul Seyfert <Paul.Seyfert@cern.ch>
 * @date 2017-10-17
 *
 * @brief unit test for constructors
 */

#include <tuple>
#include <vector>
#include <iostream>

#include "SOAContainer.h"
#include "gtest/gtest.h"

namespace AOS {
    class Point {
    private:
        float m_x;
        float m_y;

    public:
        Point(float x, float y) : m_x(x), m_y(y) {}
        // some constructors which don't just list all members
        Point() : m_x(0.f), m_y(0.f) {}
        Point(float y) : m_x(1.f), m_y(y) {}
        float x() const noexcept { return m_x; }
        float y() const noexcept { return m_y; }
        void setX(float x) noexcept { m_x = x; }
        void setY(float y) noexcept { m_y = y; }
        // plus some routines that do more than just setting/getting members
        float r2() const noexcept { return m_x * m_x + m_y * m_y; }
    };

    std::ostream& operator<<(std::ostream& os, const Point& point)
    {
        return os << "{" << point.x() << ", " << point.y() << "}";
    }

    typedef std::vector<Point> Points;
    typedef Point& AOSPoint;
} // namespace AOS

namespace SOA {
    namespace PointFields {
        using namespace SOA::Typelist;
        // since we can have more than one member of the same type in our
        // SOA object, we have to do some typedef gymnastics so the compiler
        // can tell them apart
        struct x : wrap_type<float> {};
        struct y : wrap_type<float> {};
    } // namespace PointFields

    template <typename NAKEDPROXY>
    class SOAPointProxy : public SOA::PrintableNullSkin<NAKEDPROXY> {
    public:
        using PrintableNullSkin<NAKEDPROXY>::PrintableNullSkin;
        using PrintableNullSkin<NAKEDPROXY>::operator=;
        using fields_typelist =
                SOA::Typelist::typelist<PointFields::x, PointFields::y>;

        // some constructors which don't just list all members
        SOAPointProxy()
                : SOA::PrintableNullSkin<NAKEDPROXY>(0.f, // x
                                                     0.f  // y
                  )
        {}
        SOAPointProxy(float y)
                : SOA::PrintableNullSkin<NAKEDPROXY>(1.f, // x
                                                     y)
        {}
        float x() const noexcept
        {
            return this->template get<PointFields::x>();
        }
        float y() const noexcept
        {
            return this->template get<PointFields::y>();
        }
        void setX(float x) noexcept
        {
            this->template get<PointFields::x>() = x;
        }
        void setY(float y) noexcept
        {
            this->template get<PointFields::y>() = y;
        }

        // again, something beyond plain setters/getters
        float r2() const noexcept { return x() * x() + y() * y(); }
    };

    // define the SOA container type
    typedef SOA::Container<std::vector,   // underlying type for each field
                           SOAPointProxy> // skin to "dress" the tuple of
                                          // fields with
                                                  Points;
    // define the SOAPoint itself
    typedef typename Points::proxy Point;
} // namespace SOA

TEST(SOAContainerVector, EmplaceBack)
{
    AOS::Points aospoints;
    aospoints.emplace_back();     // should add 0,0
    aospoints.emplace_back(2.f);  // should add 1,2
    aospoints.emplace_back(4, 5); // should add 4,5

    SOA::Points soapoints;
    soapoints.emplace_back();     // should add 0,0
    soapoints.emplace_back(2.f);  // should add 1,2
    soapoints.emplace_back(4, 5); // should add 4,5

    EXPECT_EQ(soapoints.size(), aospoints.size());

    for (size_t i = 0; i < soapoints.size(); ++i) {
        EXPECT_EQ(soapoints[i].x(), aospoints[i].x());
        EXPECT_EQ(soapoints[i].y(), aospoints[i].y());
    }
}
