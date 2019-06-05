/** @file tests/SOAAlgorithms.cc
 *
 * @brief test SOA algorithms
 *
 * @author Manuel Schiller <Manuel.Schiller@glasgow.ac.uk>
 * @date 2019-06-05
 *
 * For copyright and license information, see the end of the file.
 */

#include "gtest/gtest.h"
#include "SOAAlgorithms.h"

namespace Fields {
    SOAFIELD_TRIVIAL(f_x, x, float);
    SOAFIELD_TRIVIAL(f_y, y, float);
    SOAFIELD_TRIVIAL(f_n, n, int);
    SOASKIN_TRIVIAL(SkinUnique, f_x, f_n);
    SOASKIN_TRIVIAL(SkinNonUnique, f_x, f_y, f_n);
}

TEST(SOAAlgorithms, TransformUniqueFields) {
    using namespace Fields;
    SOA::Container<std::vector, SkinUnique> c1;
    c1.emplace_back(0.f, 3);
    c1.emplace_back(1.f, 2);
    c1.emplace_back(2.f, 1);
    c1.emplace_back(3.f, 0);
    auto c2 = transform(c1, [](SOA::cref<f_n> n, SOA::cref<f_x> x) {
        return SOA::value<f_n>(n + x);
    });
    EXPECT_EQ(4, c2.size());
    EXPECT_EQ(3, c2[0].n());
    EXPECT_EQ(3, c2[1].n());
    EXPECT_EQ(3, c2[2].n());
    EXPECT_EQ(3, c2[3].n());
    auto c3 = transform(c1, [](int n, const float& x) {
        return SOA::value<f_n>(n + x);
    });
    EXPECT_EQ(4, c3.size());
    EXPECT_EQ(3, c3[0].n());
    EXPECT_EQ(3, c3[1].n());
    EXPECT_EQ(3, c3[2].n());
    EXPECT_EQ(3, c3[3].n());
}

TEST(SOAAlgorithms, TransformNonUniqueFields) {
    using namespace Fields;
    SOA::Container<std::vector, SkinNonUnique> c1;
    c1.emplace_back(0.f, 2.f, 3);
    c1.emplace_back(1.f, 3.f, 2);
    c1.emplace_back(2.f, 4.f, 1);
    c1.emplace_back(3.f, 5.f, 0);
    auto c2 = transform(c1, [](SOA::cref<f_y> y, SOA::cref<f_n> n, SOA::cref<f_x> x) {
        return SOA::value<f_n>(y * (n + x));
    });
    EXPECT_EQ(4, c2.size());
    EXPECT_EQ( 6, c2[0].n());
    EXPECT_EQ( 9, c2[1].n());
    EXPECT_EQ(12, c2[2].n());
    EXPECT_EQ(15, c2[3].n());
}

TEST(SOAAlgorithms, TransformNonUniqueFieldsMultipleReturn) {
    using namespace Fields;
    SOA::Container<std::vector, SkinNonUnique> c1;
    c1.emplace_back(0.f, 2.f, 3);
    c1.emplace_back(1.f, 3.f, 2);
    c1.emplace_back(2.f, 4.f, 1);
    c1.emplace_back(3.f, 5.f, 0);
    auto c2 = transform(c1, [](SOA::cref<f_y> y, SOA::cref<f_n> n, SOA::cref<f_x> x) {
        return std::make_tuple(SOA::value<f_n>(y * (n + x)), y);
    });
    EXPECT_EQ(4, c2.size());
    EXPECT_EQ( 6, c2[0].n());
    EXPECT_EQ(2.f, c2[0].y());
    EXPECT_EQ( 9, c2[1].n());
    EXPECT_EQ(3.f, c2[1].y());
    EXPECT_EQ(12, c2[2].n());
    EXPECT_EQ(4.f, c2[2].y());
    EXPECT_EQ(15, c2[3].n());
    EXPECT_EQ(5.f, c2[3].y());
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

// vim: sw=4:ft=cpp:et:tw=78
