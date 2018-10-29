/** @file Branchless.cc
 *
 * unit test for branchless programming by providing a branchless substitute
 * of the ternary operator, "cond ? vTrue : vFalse"
 *
 * @date 2016-02-22
 * @author Manuel Schiller <Manuel.Schiller@cern.ch>
 *
 * For copyright and license information, see the end of the file.
 */
#include "gtest/gtest.h"

#include "Branchless.h"

#include <cstdio>
#include <chrono>
#include <cstdlib>

TEST(Branchless, SimpleTests) {
    struct point {
        float m_x, m_y, m_z, m_w;
        point(float x = 0.f, float y = 0.f, float z = 0.f, float w = 0.f) : m_x(x), m_y(y), m_z(z), m_w(w) { }
    };

    constexpr unsigned size = 1 << 12;
    point p[size] __attribute__((aligned(16)));
    point q[size] __attribute__((aligned(16)));
    point r[size] __attribute__((aligned(16)));
    for (unsigned i = 0; i < size; ++i) {
        p[i].m_x = 2. * drand48() - 1.;
        p[i].m_y = 2. * drand48() - 1.;
        p[i].m_z = 2. * drand48() - 1.;
        p[i].m_w = 2. * drand48() - 1.;
    }
    auto t0 = std::chrono::high_resolution_clock::now();
    for (unsigned i = 0; i < size; ++i) {
        q[i].m_x = (p[i].m_x < 0) ? p[i].m_w : p[i].m_x;
        q[i].m_y = (p[i].m_x < 0) ? p[i].m_x : p[i].m_y;
        q[i].m_z = (p[i].m_x < 0) ? p[i].m_y : p[i].m_z;
        q[i].m_w = (p[i].m_x < 0) ? p[i].m_z : p[i].m_w;
    }
    auto t1 = std::chrono::high_resolution_clock::now();
    unsigned flips1 = 0;
    for (unsigned i = 0; i < size; ++i) {
        flips1 += p[i].m_x != q[i].m_x;
        flips1 += p[i].m_y != q[i].m_y;
        flips1 += p[i].m_z != q[i].m_z;
        flips1 += p[i].m_w != q[i].m_w;
    };
    auto t2 = std::chrono::high_resolution_clock::now();
    for (unsigned i = 0; i < size; ++i) {
        r[i].m_x = sel(p[i].m_x < 0, p[i].m_w, p[i].m_x);
        r[i].m_y = sel(p[i].m_x < 0, p[i].m_x, p[i].m_y);
        r[i].m_z = sel(p[i].m_x < 0, p[i].m_y, p[i].m_z);
        r[i].m_w = sel(p[i].m_x < 0, p[i].m_z, p[i].m_w);
    }
    auto t3 = std::chrono::high_resolution_clock::now();
    unsigned flips2 = 0;
    for (unsigned i = 0; i < size; ++i) {
        flips2 += p[i].m_x != r[i].m_x;
        flips2 += p[i].m_y != r[i].m_y;
        flips2 += p[i].m_z != r[i].m_z;
        flips2 += p[i].m_w != r[i].m_w;
    };
    EXPECT_EQ(flips1, flips2);
    for (unsigned i = 0; i < size; ++i) {
        EXPECT_EQ(q[i].m_w, r[i].m_w);
        EXPECT_EQ(q[i].m_x, r[i].m_x);
        EXPECT_EQ(q[i].m_y, r[i].m_y);
        EXPECT_EQ(q[i].m_z, r[i].m_z);
    }
    std::chrono::duration<double> dt0 = t1 - t0, dt1 = t3 - t2;
    const double conv = 1e9 / double(size);
    // make sure the performance doesn't suck too much...
    EXPECT_LT(conv * dt1.count(), 3 * conv * dt0.count());
#if 0
    std::printf("branchful loop : %g ns/input value\n", conv * dt0.count());
    std::printf("branchless loop: %g ns/input value\n", conv * dt1.count());
#endif
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
 */

// vim: sw=4:tw=78:ft=cpp
