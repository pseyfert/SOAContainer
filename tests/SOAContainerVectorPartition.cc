/** @file tests/SOAContainerVectorPartition.cc
 *
 * @brief check that std::partition works
 *
 * @author Manuel Schiller <Manuel.Schiller@cern.ch>
 * @date 2015-04-11
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

// vim: sw=4:tw=78:ft=cpp:et
