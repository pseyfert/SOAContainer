/** @file tests/SOAContainerArrayAsField.cc
 *
 * @brief very simple test to test an std::array as field
 *
 * @author Manuel Schiller <Manuel.Schiller@cern.ch>
 * @date 2015-04-11
 *
 * For copyright and license information, see the end of the file.
 */

#include <tuple>
#include <array>
#include <vector>

#include "gtest/gtest.h"
#include "SOAContainer.h"

namespace stdarraytest_fields {
    typedef std::array<unsigned, 16> Array;
    typedef SOA::Typelist::wrap_type<Array> f_array;

    template <typename NAKEDPROXY>
    struct ContainerSkin : SOA::PrintableNullSkin<NAKEDPROXY> {
        // define fields, forward to base class constructors where possible
        using fields_typelist = SOA::Typelist::typelist<f_array>;
        using SOA::PrintableNullSkin<NAKEDPROXY>::PrintableNullSkin;
        using SOA::PrintableNullSkin<NAKEDPROXY>::operator=;

        /// Array type to use
        using Array = stdarraytest_fields::Array;

        /// stupid constructor from an (ignored) bool
        ContainerSkin(bool b) : ContainerSkin(Array{{b}})
        { }
        const Array& arr() const { return this->template get<f_array>(); }
    };

    using SOAArray = SOA::Container<std::vector, ContainerSkin, f_array>;
}

TEST(SOAContainerVector, ArrayAsField) {
    using namespace stdarraytest_fields;
    SOAArray a;
    a.push_back(SOAArray::value_type(true));
    a.emplace_back(SOAArray::value_type(false));
    a.emplace_back(true);
    EXPECT_EQ(a.size(), 3u);
    EXPECT_EQ(a[0].arr()[0], 1);
    EXPECT_EQ(a[1].arr()[0], 0);
    EXPECT_EQ(a[2].arr()[0], 1);
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

// vim: sw=4:tw=78:ft=cpp:et
