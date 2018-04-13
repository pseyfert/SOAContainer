/** @file tests/SOAContainerVectorZipViews.cc
 *
 * @brief check that zipping together views works
 *
 * @author Manuel Schiller <Manuel.Schiller@cern.ch>
 * @date 2015-04-11
 */

#include <cmath>
#include <tuple>
#include <vector>

#include "SOAContainer.h"
#include "gtest/gtest.h"

namespace FieldExtractionTest {
    SOAFIELD_TRIVIAL(f_x, x, float);
    SOAFIELD_TRIVIAL(f_y, y, float);
    SOAFIELD_TRIVIAL(f_z, z, float);
    SOASKIN_TRIVIAL(Point, f_x, f_y, f_z);
    SOASKIN(RPhiSkin, f_x, f_y)
    {
        SOASKIN_INHERIT_DEFAULT_METHODS(RPhiSkin);
        float r() const noexcept
        {
            return std::sqrt(this->x() * this->x() + this->y() * this->y());
        }
        float phi() const noexcept
        {
            return std::atan2(this->y(), this->x());
        }
    };
} // namespace FieldExtractionTest

TEST(SOAView, ZipViews)
{
    using namespace FieldExtractionTest;
    const auto rnd = []() { return double(random()) / double(RAND_MAX); };
    SOA::Container<std::vector, Point> c;
    // fill the container
    c.reserve(16);
    for (unsigned i = 0; i < 16; ++i) c.emplace_back(rnd(), rnd(), rnd());
    EXPECT_EQ(c.size(), 16u);
    // test extraction of some fields into a new view
    auto v1 = c.view<f_x>();
    auto v2 = c.view<f_y>();
    auto v3 = c.view<f_z>();
    auto v4 = zip(v1, v2, v3);
    EXPECT_EQ(c.size(), v4.size());
    for (unsigned i = 0; i < c.size(); ++i) {
        EXPECT_EQ(c[i].x(), v4[i].x());
        EXPECT_EQ(c[i].y(), v4[i].y());
        EXPECT_EQ(c[i].z(), v4[i].z());
    }
    auto rphi = zip<RPhiSkin>(v1, v2);
    EXPECT_EQ(c.size(), rphi.size());
    for (unsigned i = 0; i < c.size(); ++i) {
        EXPECT_FLOAT_EQ(std::sqrt(c[i].x() * c[i].x() + c[i].y() * c[i].y()),
                        rphi[i].r());
        EXPECT_FLOAT_EQ(std::atan2(c[i].y(), c[i].x()), rphi[i].phi());
    }
}

// vim: sw=4:tw=78:ft=cpp:et
