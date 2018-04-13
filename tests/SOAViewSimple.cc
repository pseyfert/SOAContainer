/** @file tests/SOAViewSimple.cc
 *
 * @brief very simple test to create a SOA::View from raw containers
 *
 * @author Manuel Schiller <Manuel.Schiller@cern.ch>
 * @date 2015-04-11
 */

#include <cmath>
#include <tuple>
#include <vector>

#include "gtest/gtest.h"
#include "SOAView.h"

typedef struct : SOA::Typelist::wrap_type<float> {} field_x;
typedef struct : SOA::Typelist::wrap_type<float> {} field_y;
template <typename NAKEDPROXY>
class SOAPoint : public NAKEDPROXY {
    public:
        using fields_typelist = SOA::Typelist::typelist<field_x, field_y>;
        using NAKEDPROXY::NAKEDPROXY;
        using NAKEDPROXY::operator=;

        float x() const noexcept
        { return this-> template get<field_x>(); }
        float y() const noexcept
        { return this-> template get<field_y>(); }
        float& x() noexcept
        { return this-> template get<field_x>(); }
        float& y() noexcept
        { return this-> template get<field_y>(); }
        float r2() const noexcept { return x() * x() + y() * y(); }
};

TEST (SOAView, Simple) {
    std::vector<float> vx, vy, vxx, vyy;
    // fill vx, vy somehow - same number of elements
    const auto rnd = [] () { return double(random()) / double(RAND_MAX); };
    vx.reserve(1024), vy.reserve(1024);
    for (unsigned i = 0; i < 1024; ++i) {
        vx.push_back(rnd());
        vy.push_back(rnd());
    }
    vxx = vx, vyy = vy;
    // construct a View from vx, vy
    auto view = SOA::make_soaview<SOAPoint>(vx, vy);
    // data must look the same
    EXPECT_EQ(vx.front(), view.front().x());
    EXPECT_EQ(vy.front(), view.front().y());
    // and be at the same address
    EXPECT_EQ(&vx.front(), &*view.begin<field_x>());
    EXPECT_EQ(&vy.front(), &*view.begin<field_y>());
    const float angle = 42.f / 180.f * M_PI;
    const auto s = std::sin(angle), c = std::cos(angle);
    for (auto p: view) {
        if (p.r2() > 1) continue;
        // rotate points within the unit circle by given angle
        std::tie(p.x(), p.y()) = std::make_pair(
                c * p.x() + s * p.y(), -s * p.x() + c * p.y());
    }
    // do the same thing "by hand" on vxx, vyy
    for (unsigned i = 0; i < 1024; ++i) {
        if ((vxx[i] * vxx[i] + vyy[i] * vyy[i]) > 1) continue;
        // rotate points within the unit circle by given angle
        std::tie(vxx[i], vyy[i]) = std::make_pair(
                c * vxx[i] + s * vyy[i], -s * vxx[i] + c * vyy[i]);
    }
    // check that we get the same results in both cases
    EXPECT_EQ(view.size(), vxx.size());
    EXPECT_EQ(view.size(), vyy.size());
    unsigned i = 0;
    for (auto p: view) {
        EXPECT_LT(std::abs(p.x() - vxx[i]), 64 *
                std::numeric_limits<float>::epsilon() *
                std::max(std::abs(p.x()), std::abs(vxx[i])));
        EXPECT_LT(std::abs(p.y() - vyy[i]), 64 *
                std::numeric_limits<float>::epsilon() *
                std::max(std::abs(p.y()), std::abs(vyy[i])));
        ++i;
    }
    // check that we can access the underlying ranges
    auto rx = view.range<field_x>();
    EXPECT_EQ(std::size_t(rx.size()), std::size_t(view.size()));
    EXPECT_EQ(&vx.front(), &rx.front());
    // check subranges
    auto rxsub = view.range<field_x>(view.begin() + 1, view.end());
    EXPECT_EQ(std::size_t(rxsub.size()) + 1, std::size_t(view.size()));
    EXPECT_EQ(&*(vx.begin() + 1), &rxsub.front());
}

// vim: sw=4:tw=78:ft=cpp:et
