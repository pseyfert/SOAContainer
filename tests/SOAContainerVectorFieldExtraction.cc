/** @file tests/SOAContainerVectorFieldExtraction.cc
 *
 * @brief check that extraction of fields with .view<...>() works
 *
 * @author Manuel Schiller <Manuel.Schiller@cern.ch>
 * @date 2015-04-11
 *
 * For copyright and license information, see the end of the file.
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

TEST(SOAContainerVector, FieldExtraction)
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
    EXPECT_EQ(c.size(), v1.size());
    for (unsigned i = 0; i < c.size(); ++i) {
        EXPECT_EQ(c[i].x(), v1[i].x());
    }
    auto v2 = c.view<f_y>();
    EXPECT_EQ(c.size(), v2.size());
    for (unsigned i = 0; i < c.size(); ++i) {
        EXPECT_EQ(c[i].y(), v2[i].y());
    }
    auto v3 = c.view<f_z>();
    EXPECT_EQ(c.size(), v3.size());
    for (unsigned i = 0; i < c.size(); ++i) {
        EXPECT_EQ(c[i].z(), v3[i].z());
    }
    auto v4 = c.view<f_x, f_y>();
    EXPECT_EQ(c.size(), v4.size());
    for (unsigned i = 0; i < c.size(); ++i) {
        EXPECT_EQ(c[i].x(), v4[i].x());
        EXPECT_EQ(c[i].y(), v4[i].y());
    }
    auto v5 = c.view<f_y, f_z>();
    EXPECT_EQ(c.size(), v5.size());
    for (unsigned i = 0; i < c.size(); ++i) {
        EXPECT_EQ(c[i].y(), v5[i].y());
        EXPECT_EQ(c[i].z(), v5[i].z());
    }
    auto v6 = c.view<f_x, f_z>();
    EXPECT_EQ(c.size(), v6.size());
    for (unsigned i = 0; i < c.size(); ++i) {
        EXPECT_EQ(c[i].x(), v6[i].x());
        EXPECT_EQ(c[i].z(), v6[i].z());
    }
    auto v7 = c.view<f_x, f_y, f_z>();
    EXPECT_EQ(c.size(), v7.size());
    for (unsigned i = 0; i < c.size(); ++i) {
        EXPECT_EQ(c[i].x(), v7[i].x());
        EXPECT_EQ(c[i].y(), v7[i].y());
        EXPECT_EQ(c[i].z(), v7[i].z());
    }
    auto rphi = c.view<RPhiSkin>();
    EXPECT_EQ(c.size(), rphi.size());
    for (unsigned i = 0; i < c.size(); ++i) {
        EXPECT_FLOAT_EQ(std::sqrt(c[i].x() * c[i].x() + c[i].y() * c[i].y()),
                        rphi[i].r());
        EXPECT_FLOAT_EQ(std::atan2(c[i].y(), c[i].x()), rphi[i].phi());
    }
}

/* Copyright (C) CERN for the benefit of the LHCb collaboration
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * In applying this licence, CERN does not waive the privileges and immunities
 * granted to it by virtue of its status as an Intergovernmental Organization
 * or submit itself to any jurisdiction.
 */

// vim: sw=4:tw=78:ft=cpp:et
