/** @file tests/SOAZipTestPaul.cc
 *
 * @brief test a use-reference-after-out-of-scope bug in zip
 *
 * @author Paul Seyfert <Paul.Seyfert@cern.ch>
 * @date 2019-03-07
 *
 * For copyright and license information, see the end of the file.
 */

#include <cmath>
#include <cstdlib>
#include <tuple>
#include <vector>

#include "SOAContainer.h"
#include "gtest/gtest.h"

namespace {
    struct hit {
        float x;
        float y;
        float z;
    };

    struct track {
        std::vector<hit> hits;
    };

    SOAFIELD_TRIVIAL(f_track, track_accessor, track);
    SOASKIN_TRIVIAL(track_skin, f_track);
} // namespace

TEST(SOAZipTest, Paul)
{
    const auto rnd = []() { return float(random()) / float(RAND_MAX); };
    SOA::Container<std::vector, track_skin> c;
    // fill the container
    c.reserve(4);
    for (unsigned i = 0; i < 4; ++i) {
        c.emplace_back();
        for (unsigned j = 0; j < 8; ++j) {
            c.back().track_accessor().hits.emplace_back(
                    hit{rnd(), rnd(), rnd()});
        }
    }

    auto v = zip<track_skin>(c);
    EXPECT_EQ(c.size(), v.size());
    for (unsigned i = 0; i < c.size(); ++i) {
        for (unsigned j = 0; j < c[i].track_accessor().hits.size(); ++j) {
            EXPECT_FLOAT_EQ(c[i].track_accessor().hits[j].x,
                            v[i].track_accessor().hits[j].x);
        }
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
