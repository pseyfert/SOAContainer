
#include <iostream>
#include <array>
#include <vector>
#include <tuple>

#include "SOAContainer.h"


namespace classic {

class Point {
    private:
        float m_x;
        float m_y;
    public:
    Point(float x, float y) : m_x(x), m_y(y) { }
    float x() const noexcept { return m_x; }
    float y() const noexcept { return m_y; }
        void setX(float x) noexcept { m_x = x; }
        void setY(float y) noexcept { m_y = y; }
        // plus some routines that do more than just setting/getting members
    float r2() const noexcept { return m_x * m_x + m_y * m_y; }
};

std::ostream& operator<<(std::ostream& os, const Point& point) {
    return os << "[" << point.x() << ", " << point.y() << "]";
}

typedef std::vector<Point> AOSPoints;
typedef Point& AOSPoint; 

};

namespace modern {
namespace PointFields {
    using namespace SOATypelist;
    // since we can have more than one member of the same type in our
    // SOA object, we have to do some typedef gymnastics so the compiler
    // can tell them apart
    typedef struct : public wrap_type<float> { } x;
    typedef struct : public wrap_type<float> { } y;
};


template <typename NAKEDPROXY>
class SOAPointProxy : public NAKEDPROXY {
    public:
        /// forward constructor to NAKEDPROXY's constructor
        template <typename... ARGS>
        SOAPointProxy(ARGS&&... args) :
            NAKEDPROXY(std::forward<ARGS>(args)...) { }

        /// assignment operator - forward to underlying proxy
        template <typename ARG>
        SOAPointProxy<NAKEDPROXY>& operator=(const ARG& arg)
        { NAKEDPROXY::operator=(arg); return *this; }

        /// move assignment operator - forward to underlying proxy
        template <typename ARG>
        SOAPointProxy<NAKEDPROXY>& operator=(ARG&& arg)
        { NAKEDPROXY::operator=(std::move(arg)); return *this; }

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
typedef SOAContainer<
        std::vector, // underlying type for each field
        SOAPointProxy,    // skin to "dress" the tuple of fields with
        // one or more wrapped types which each tag a member/field
        PointFields::x, PointFields::y> SOAPoints;
// define the SOAPoint itself
typedef typename SOAPoints::proxy Point;
};

int main() {
    using namespace std;
    {   
        using namespace classic;
        cout << "This is a normal array of structures" << endl;
        AOSPoints list_of_points = {Point(1,2), Point(2,3), Point(3,4)};

        cout << list_of_points.at(0) << endl;
        cout << list_of_points.at(1) << endl;
        cout << list_of_points.at(2) << endl;
        
        cout << "we can access using list_of_points.at(1).x(): " << list_of_points.at(1).x() << endl;
    }

    {   
        using namespace modern;

        cout << endl << "This is a SOA wrapper:" << endl;

        //SOAPoints list_of_points = {Point(1,2), Point(2,3), Point(3,4)};
        //SOAPoints list_of_points = {{1,2}, {2, 3}, {3, 4}};

        SOAPoints list_of_points;

        //list_of_points.push_back(Point(1,2));
        //list_of_points.push_back(Point(1,2));
        //cout << list_of_points.at(0) << endl;
        //cout << list_of_points.at(1) << endl;
        //cout << list_of_points.at(2) << endl;
        
        cout << "we can access using list_of_points.at(1).x(): " << list_of_points.at(1).x() << endl;

    }
    return 0;
}
