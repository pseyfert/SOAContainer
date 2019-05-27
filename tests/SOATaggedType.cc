/** @file tests/SOATaggedType.cc
 *
 * @brief test tagged types
 *
 * @author Manuel Schiller <Manuel.Schiller@glasgow.ac.uk>
 * @date 2019-05-21
 *
 * For copyright and license information, see the end of the file.
 */

#include "SOATaggedType.h"
#include "SOAField.h"

#include "gtest/gtest.h"

namespace TrivialArithFloatFields {
    SOAFIELD_TRIVIAL(f_x, x, float);
    SOAFIELD_TRIVIAL(f_y, y, float);
}
namespace TrivialArithIntFields {
    SOAFIELD_TRIVIAL(f_x, x, short);
}

TEST(SOATaggedType, TrivialArithFloatByValue)
{
    using namespace TrivialArithFloatFields;
    SOA::value<f_x> f(40.f);
    static_assert(sizeof(f) == sizeof(float), "SOA::value has wrong size");
    EXPECT_EQ(f, 40.f);
    EXPECT_EQ(f.x(), 40.f);
    f.x() = 41.f;
    EXPECT_EQ(f, 41.f);
    EXPECT_EQ(f.x(), 41.f);
    f += 1;
    EXPECT_EQ(f, 42.f);
    f /= 2;
    EXPECT_EQ(f, 21.f);
    f -= 1;
    EXPECT_EQ(f, 20.f);
    f *= 2;
    EXPECT_EQ(f, 40.f);
    float x0 = ++f;
    EXPECT_EQ(f, 41.f);
    EXPECT_EQ(x0, 41.f);
    float x1 = f++;
    EXPECT_EQ(x1, 41.f);
    EXPECT_EQ(f, 42.f);
    float x2 = --f;
    EXPECT_EQ(f, 41.f);
    EXPECT_EQ(x2, 41.f);
    float x3 = f--;
    EXPECT_EQ(f, 40.f);
    EXPECT_EQ(x3, 41.f);
}

TEST(SOATaggedType, TrivialArithFloatByRef)
{
    using namespace TrivialArithFloatFields;
    float x = 40.f;
    SOA::ref<f_x> f(x);
    static_assert(sizeof(f) == sizeof(float*), "SOA::ref has wrong size");
    EXPECT_EQ(f, 40.f);
    EXPECT_EQ(f.x(), 40.f);
    f.x() = 41.f;
    EXPECT_EQ(f, 41.f);
    EXPECT_EQ(f.x(), 41.f);
    f += 1;
    EXPECT_EQ(f, 42.f);
    f /= 2;
    EXPECT_EQ(f, 21.f);
    f -= 1;
    EXPECT_EQ(f, 20.f);
    f *= 2;
    EXPECT_EQ(f, 40.f);
    float x0 = ++f;
    EXPECT_EQ(f, 41.f);
    EXPECT_EQ(x0, 41.f);
    float x1 = f++;
    EXPECT_EQ(x1, 41.f);
    EXPECT_EQ(f, 42.f);
    float x2 = --f;
    EXPECT_EQ(f, 41.f);
    EXPECT_EQ(x2, 41.f);
    float x3 = f--;
    EXPECT_EQ(f, 40.f);
    EXPECT_EQ(x3, 41.f);
    EXPECT_EQ(x, 40.f);
    x = 42.f;
    EXPECT_EQ(f, 42.f);
}

TEST(SOATaggedType, TrivialArithFloatByCref)
{
    using namespace TrivialArithFloatFields;
    const float x = 41.f;
    SOA::cref<f_x> f(x);
    static_assert(sizeof(f) == sizeof(const float*),
                  "SOA::cref has wrong size");
    EXPECT_EQ(f, 41.f);
    EXPECT_EQ(f.x(), 41.f);
}

TEST(SOATaggedType, TrivialArithIntByValue)
{
    using namespace TrivialArithIntFields;
    SOA::value<f_x> f(40);
    static_assert(sizeof(f) == sizeof(short), "SOA::value has wrong size");
    EXPECT_EQ(f, 40);
    EXPECT_EQ(f.x(), 40);
    f.x() = 41;
    EXPECT_EQ(f, 41);
    EXPECT_EQ(f.x(), 41);
    f += 1;
    EXPECT_EQ(f, 42);
    f /= 2;
    EXPECT_EQ(f, 21);
    f -= 1;
    EXPECT_EQ(f, 20);
    f *= 2;
    EXPECT_EQ(f, 40);
    float x0 = ++f;
    EXPECT_EQ(f, 41);
    EXPECT_EQ(x0, 41);
    float x1 = f++;
    EXPECT_EQ(x1, 41);
    EXPECT_EQ(f, 42);
    float x2 = --f;
    EXPECT_EQ(f, 41);
    EXPECT_EQ(x2, 41);
    float x3 = f--;
    EXPECT_EQ(f, 40);
    EXPECT_EQ(x3, 41);
    f %= 17;
    EXPECT_EQ(f, 6);
    f >>= 1;
    EXPECT_EQ(f, 3);
    f &= 2;
    EXPECT_EQ(f, 2);
    f |= 1;
    EXPECT_EQ(f, 3);
    f ^= 1;
    EXPECT_EQ(f, 2);
    f ^= 1;
    EXPECT_EQ(f, 3);
    f <<= 1;
    EXPECT_EQ(f, 6);
}

TEST(SOATaggedType, TrivialArithIntByRef)
{
    using namespace TrivialArithIntFields;
    short x = 40;
    SOA::ref<f_x> f(x);
    static_assert(sizeof(f) == sizeof(short*), "SOA::ref has wrong size");
    EXPECT_EQ(f, 40);
    EXPECT_EQ(f.x(), 40);
    f.x() = 41;
    EXPECT_EQ(f, 41);
    EXPECT_EQ(f.x(), 41);
    f += 1;
    EXPECT_EQ(f, 42);
    f /= 2;
    EXPECT_EQ(f, 21);
    f -= 1;
    EXPECT_EQ(f, 20);
    f *= 2;
    EXPECT_EQ(f, 40);
    float x0 = ++f;
    EXPECT_EQ(f, 41);
    EXPECT_EQ(x0, 41);
    float x1 = f++;
    EXPECT_EQ(x1, 41);
    EXPECT_EQ(f, 42);
    float x2 = --f;
    EXPECT_EQ(f, 41);
    EXPECT_EQ(x2, 41);
    float x3 = f--;
    EXPECT_EQ(f, 40);
    EXPECT_EQ(x3, 41);
    f %= 17;
    EXPECT_EQ(f, 6);
    f >>= 1;
    EXPECT_EQ(f, 3);
    f &= 2;
    EXPECT_EQ(f, 2);
    f |= 1;
    EXPECT_EQ(f, 3);
    f ^= 1;
    EXPECT_EQ(f, 2);
    f ^= 1;
    EXPECT_EQ(f, 3);
    f <<= 1;
    EXPECT_EQ(f, 6);
    EXPECT_EQ(x, 6);
    x = 7;
    EXPECT_EQ(f, 7);
}

TEST(SOATaggedType, TrivialArithIntByCref)
{
    using namespace TrivialArithIntFields;
    const short x = 41;
    SOA::cref<f_x> f(x);
    static_assert(sizeof(f) == sizeof(const short*),
                  "SOA::cref has wrong size");
    EXPECT_EQ(f, 41);
    EXPECT_EQ(f.x(), 41);
}

#include "SOAContainer.h"

namespace TrivialArithFloatFields {
    SOASKIN_TRIVIAL(TwoFloats, f_x, f_y);
}

TEST(SOATaggedType, ConstructFromTaggedFields)
{
    using namespace TrivialArithFloatFields;
    using C = SOA::Container<std::vector, TwoFloats>;
    C::value_type el(SOA::value<f_y>(1.f), SOA::value<f_x>(0.f));
    EXPECT_EQ(el.x(), 0.f);
    EXPECT_EQ(el.y(), 1.f);
    C c;
    c.emplace_back(SOA::value<f_x>(0.f), SOA::value<f_y>(1.f));
    c.emplace_back(SOA::value<f_y>(3.f), SOA::value<f_x>(2.f));
    EXPECT_EQ(c.front().x(), 0.f);
    EXPECT_EQ(c.front().y(), 1.f);
    EXPECT_EQ(c.back().x(), 2.f);
    EXPECT_EQ(c.back().y(), 3.f);
    auto it = c.emplace(c.begin(), SOA::value<f_y>(5.f), SOA::value<f_x>(4.f));
    EXPECT_EQ(c.size(), 3u);
    EXPECT_EQ(it, c.begin());
    EXPECT_EQ(c[0].x(), 4.f);
    EXPECT_EQ(c[0].y(), 5.f);
    EXPECT_EQ(c[1].x(), 0.f);
    EXPECT_EQ(c[1].y(), 1.f);
    EXPECT_EQ(c[2].x(), 2.f);
    EXPECT_EQ(c[2].y(), 3.f);
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
