/** @file ContainerTest.cc
 *
 * @author Manuel Schiller <Manuel.Schiller@cern.ch>
 * @date 2015-04-11
 *
 * unit test for Container class
 */

#include <numeric>
#include <array>
#include <tuple>
#include <cmath>
#include <cstdlib>
#include <deque>

#include "SOAContainer.h"
#include "PrintableNullSkin.h"
#include "gtest/gtest.h"




namespace ConvenientContainersTest_Fields {
    SOAFIELD_TRIVIAL(f_x, x, float);
    SOAFIELD_TRIVIAL(f_y, y, float);
    SOAFIELD(f_flags, int,
        SOAFIELD_ACCESSORS(flags)
        enum Flag { Used = 0x1, Dead = 0x2 };
        bool isUsed() const { return flags() & Used; }
        bool isDead() const { return flags() & Dead; }
        bool setUsed(bool newState = true)
        {
            int retVal = flags();
            flags() = (retVal & ~Used) | (-newState & Used);
            return retVal & Used;
        }
        bool setDead(bool newState = true)
        {
            int retVal = flags();
            flags() = (retVal & ~Dead) | (-newState & Dead);
            return retVal & Dead;
        }
        void printflags() { std::printf("flags: %08x\n", flags()); }
    );

    SOASKIN_TRIVIAL(SkinSimple, f_x, f_y, f_flags);
    SOASKIN(Skin, f_x, f_y, f_flags) {
        SOASKIN_INHERIT_DEFAULT_METHODS(Skin);
        // special constructors go here...
        // special accessors go here...
        void setDeadIfTooFarOut()
        {
            // inside the skin, this->accessor() is required to make C++ find the
            // routine, users of the skin can just call accessor()
            auto x = this->x(), y = this->y();
            if ((x * x + y + y) > 1.f) this->setDead();
        }
    };
    // little dummy struct used to check the size behaviour of skins
    struct Foo {
        int i;
        // make SOA::impl::SkinBase happy
        template <std::size_t> void get() const noexcept {}
    };
}
TEST(Container, ConvenientContainers) {
    using namespace ConvenientContainersTest_Fields;

    // start by testing that skins of convenient containers are well-behaved,
    // i.e. the storage size is minimal, and there's no difference to stupid
    // SOA containers
    SOA::Container<std::vector, SOA::NullSkin, float, float, int> s;
    SOA::Container<std::vector, SkinSimple> csimple;
    static_assert(sizeof(s.front()) == sizeof(csimple.front()),
            "Fancy field and old-style field proxies need to have same size.");
    SOA::Container<std::vector, Skin> c;
    static_assert(sizeof(s.front()) == sizeof(c.front()),
            "Fancy field and old-style field proxies need to have same size.");
    SOA::impl::SkinBase<Foo, f_x, f_y, f_flags> sb;
    static_assert(sizeof(int) == sizeof(sb), "skin size behaviour is all wrong.");
    // okay, we're satisfied on that front. Check basic functionality
    EXPECT_TRUE(c.empty());
    EXPECT_TRUE(csimple.empty());
    c.push_back(std::make_tuple(3.14f, 2.79f, 42));
    csimple.push_back(std::make_tuple(3.14f, 2.79f, 42));
    EXPECT_EQ(c.size(), 1u);
    EXPECT_EQ(csimple.size(), 1u);
    EXPECT_EQ(c[0].x(), 3.14f);
    EXPECT_EQ(c[0].y(), 2.79f);
    EXPECT_EQ(c[0].flags(), 42);
    EXPECT_EQ(csimple[0].x(), 3.14f);
    EXPECT_EQ(csimple[0].y(), 2.79f);
    EXPECT_EQ(csimple[0].flags(), 42);
    c.front().setDead(false);
    csimple.front().setDead(false);
    EXPECT_EQ(c[0].x(), 3.14f);
    EXPECT_EQ(c[0].y(), 2.79f);
    EXPECT_EQ(c[0].flags(), 40);
    EXPECT_EQ(csimple[0].x(), 3.14f);
    EXPECT_EQ(csimple[0].y(), 2.79f);
    EXPECT_EQ(csimple[0].flags(), 40);
    // this won't work - constness
    // const_cast<const decltype(c)&>(c).front().setUsed();
    // check that sorting works
    c.push_back(std::make_tuple(2.79f, 3.14f, 17));
    csimple.push_back(std::make_tuple(2.79f, 3.14f, 17));
    std::sort(c.begin(), c.end(),
            [] (decltype(c)::value_const_reference a,
                decltype(c)::value_const_reference b)
            { return a.x() < b.x(); });
    std::sort(csimple.begin(), csimple.end(),
            [] (decltype(csimple)::value_const_reference a,
                decltype(csimple)::value_const_reference b)
            { return a.x() < b.x(); });
    for (std::size_t i = 0; i < std::max(c.size(), csimple.size()); ++i) {
        EXPECT_EQ(c[i].x(), csimple[i].x());
        EXPECT_EQ(c[i].y(), csimple[i].y());
        EXPECT_EQ(c[i].flags(), csimple[i].flags());
    }
}

namespace FieldExtractionTest {
    SOAFIELD_TRIVIAL(f_x, x, float);
    SOAFIELD_TRIVIAL(f_y, y, float);
    SOAFIELD_TRIVIAL(f_z, z, float);
    SOASKIN_TRIVIAL(Point, f_x, f_y, f_z);
    SOASKIN(RPhiSkin, f_x, f_y) {
        SOASKIN_INHERIT_DEFAULT_METHODS(RPhiSkin);
        float r() const noexcept
        { return std::sqrt(this->x() * this->x() + this->y() * this->y()); }
        float phi() const noexcept
        { return std::atan2(this->y(), this->x()); }
    };
}
TEST(SOAView, FieldExtraction) {
    using namespace FieldExtractionTest;
    const auto rnd = [] () { return double(random()) / double(RAND_MAX); };
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
    for (unsigned i = 0; i < c.size(); ++i)
    {
        EXPECT_FLOAT_EQ(std::sqrt(c[i].x() * c[i].x() + c[i].y() * c[i].y()),
                    rphi[i].r());
        EXPECT_FLOAT_EQ(std::atan2(c[i].y(), c[i].x()), rphi[i].phi());
    }
}

TEST(SOAView, ZipViews) {
    using namespace FieldExtractionTest;
    const auto rnd = [] () { return double(random()) / double(RAND_MAX); };
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
    for (unsigned i = 0; i < c.size(); ++i)
    {
        EXPECT_FLOAT_EQ(std::sqrt(c[i].x() * c[i].x() + c[i].y() * c[i].y()),
                    rphi[i].r());
        EXPECT_FLOAT_EQ(std::atan2(c[i].y(), c[i].x()), rphi[i].phi());
    }
}

namespace PartitionTestDesc {
    SOAFIELD_TRIVIAL(n, n, unsigned);
    SOAFIELD_TRIVIAL(m, m, unsigned);
    SOASKIN_TRIVIAL(Skin, n, m);
}

TEST(SOAContainer, Partition) {
    SOA::Container<std::vector, PartitionTestDesc::Skin> c;
    c.reserve(32);
    for (unsigned i = 0; i < 32; ++i) c.emplace_back(i, i);
    EXPECT_EQ(c.size(), 32u);
    auto isEven = [] (decltype(c)::value_const_reference a) noexcept { return !(a.n() & 1); }; 
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
