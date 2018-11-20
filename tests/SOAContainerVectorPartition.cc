/** @file tests/SOAContainerVectorPartition.cc
 *
 * @brief check that std::partition works
 *
 * @author Manuel Schiller <Manuel.Schiller@cern.ch>
 * @date 2015-04-11
 *
 * For copyright and license information, see the end of the file.
 */

#include <tuple>
#include <vector>

#include "SOAContainer.h"
#include "gtest/gtest.h"

namespace PartitionTestDesc {
    SOAFIELD_TRIVIAL(n, n, unsigned);
    SOAFIELD_TRIVIAL(m, m, unsigned);
    SOASKIN_TRIVIAL(Skin, n, m);
} // namespace PartitionTestDesc

TEST(SOAContainerVector, Partition)
{
    SOA::Container<std::vector, PartitionTestDesc::Skin> c;
    c.reserve(32);
    for (unsigned i = 0; i < 32; ++i) c.emplace_back(i, i);
    EXPECT_EQ(c.size(), 32u);
    auto isEven = [](decltype(c)::value_const_reference a) noexcept
    {
        return !(a.n() & 1);
    };
    const auto partitioned0 = std::is_partitioned(c.begin(), c.end(), isEven);
    EXPECT_EQ(partitioned0, false);
    const auto it1 = std::partition(c.begin(), c.end(), isEven);
    const auto partitioned1 = std::is_partitioned(c.begin(), c.end(), isEven);
    EXPECT_EQ(partitioned1, true);
    EXPECT_EQ(it1, c.begin() + 16);
    c.clear();
    for (unsigned i = 0; i < 32; ++i) c.emplace_back(i, i);
    EXPECT_EQ(c.size(), 32u);
    const auto partitioned2 = std::is_partitioned(c.begin(), c.end(), isEven);
    EXPECT_EQ(partitioned2, false);
    const auto it3 = std::stable_partition(c.begin(), c.end(), isEven);
    const auto partitioned3 = std::is_partitioned(c.begin(), c.end(), isEven);
    EXPECT_EQ(partitioned3, true);
    EXPECT_EQ(it3, c.begin() + 16);
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
