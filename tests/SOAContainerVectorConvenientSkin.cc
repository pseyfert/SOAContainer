/** @file tests/SOAContainerVectorConvenientSkin.cc
 *
 * @brief check that convenient skins work
 *
 * @author Manuel Schiller <Manuel.Schiller@cern.ch>
 * @date 2015-04-11
 *
 * For copyright and license information, see the end of the file.
 */

#include <tuple>
#include <vector>

#include "gtest/gtest.h"
#include "SOAContainer.h"

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

TEST(SOAContainerVector, ConvenientContainers) {
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

TEST(SOAContainerVector, TypeErasedViews)
{
    using namespace ConvenientContainersTest_Fields;
    SOA::Container<std::vector, Skin> c;
    const auto& ccr = c;
    auto vf = c.view<f_x, f_y, f_flags>();
    const bool b1 = std::is_same<
            decltype(vf),
            SOA::contiguous_view_from_fields_t<f_x, f_y, f_flags>>::value;
    EXPECT_EQ(b1, true);
    auto cvf = ccr.view<f_x, f_y, f_flags>();
    const bool b2 = std::is_same<decltype(cvf),
                                 SOA::contiguous_const_view_from_fields_t<
                                         f_x, f_y, f_flags>>::value;
    EXPECT_EQ(b2, true);
    auto vs = c.view<Skin>();
    const bool b3 =
            std::is_same<decltype(vs),
                         SOA::contiguous_view_from_skin_t<Skin>>::value;
    EXPECT_EQ(b3, true);
    auto cvs = ccr.view<Skin>();
    const bool b4 =
            std::is_same<decltype(cvs),
                         SOA::contiguous_const_view_from_skin_t<Skin>>::value;
    EXPECT_EQ(b4, true);
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
