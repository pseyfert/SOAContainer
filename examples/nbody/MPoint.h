#ifndef MPOINT_H
#define MPOINT_H

#include <cmath>


class MPoint {

public:
    MPoint(float x1, float y1, float z1,
           float px1, float py1, float pz1,
           float m1): m_x(x1), m_y(y1), m_z(z1),
           m_px(px1), m_py(py1), m_pz(pz1), m_m(m1) {}

    MPoint(const MPoint&) = default;
    MPoint(MPoint&&) = default;
    MPoint& operator=(const MPoint&) = default;
    MPoint& operator=(MPoint&&) = default;

    float dist() const { return sqrt(m_x * m_x + m_y * m_y + m_z * m_z); }

    float x() const { return m_x; }
    float y() const { return m_y; }
    float z() const { return m_z; }

    float& x() { return m_x; }
    float& y() { return m_y; }
    float& z() { return m_z; }

    void move(float dt) {
	auto mdt = dt / m_m;
        m_x +=  m_px * mdt;
        m_y +=  m_py * mdt;
        m_z +=  m_pz * mdt;
    }

    float px() const { return m_px; }
    float py() const { return m_py; }
    float pz() const { return m_pz; }

    float& px() { return m_px; }
    float& py() { return m_py; }
    float& pz() { return m_pz; }

    float m() const { return m_m; }


private:
    float m_x;
    float m_y;
    float m_z;
    float m_px;
    float m_py;
    float m_pz;
    float m_m;
};

typedef std::vector<MPoint> MPoints;

#endif // MPOINT_H
