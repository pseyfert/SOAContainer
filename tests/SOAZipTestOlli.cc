/** @file tests/SOAZipTestOlli.cc
 *
 * @brief test a foo
 *
 * @author Manuel Schiller <Manuel.Schiller@glasgow.ac.uk>
 * @date 2020-02-05
 *
 * For copyright and license information, see the end of the file.
 */

#include <cmath>
#include <cstdlib>
#include <tuple>
#include <vector>

#include "SOAContainer.h"
#include "gtest/gtest.h"

namespace Moo {
    SOAFIELD_TRIVIAL(f_moo, moo, int);
    SOASKIN_TRIVIAL(s_moo, f_moo);

    struct twoint {
        int m_i1;
        int m_i2;
    };
    SOAFIELD_TRIVIAL(f_twoint, get, twoint);
    SOASKIN_TRIVIAL(s_twoint, f_twoint);
}

template <typename C>
class Container : public C
{
public:
    using view_t = C;
    template <typename... ARGS>
    Container(ARGS&&... args) : C(std::forward<ARGS>(args)...) {}
    Container() = default;
    Container(const Container&) = default;
    Container(Container&&) = default;

#if 0
    template <typename T>
    auto push_back(const T& val) -> typename std::enable_if<
            1 == view_t::self_type::fields_typelist::size() &&
                    std::is_same<typename std::remove_cv<
                                         typename std::remove_reference<
                                                 T>::type>::type,
                                 typename view_t::self_type::fields_typelist::
                                         template at<0>::type::type>::value,
            decltype(C::emplace_back(val))>::type
    { return C::emplace_back(val); }
#endif
    template <typename T>
    auto push_back(T&& val) -> typename std::enable_if<
            1 == view_t::self_type::fields_typelist::size() &&
                    std::is_same<typename std::remove_cv<
                                         typename std::remove_reference<
                                                 T>::type>::type,
                                 typename view_t::self_type::fields_typelist::
                                         template at<0>::type::type>::value,
            decltype(C::emplace_back(std::forward<T>(val)))>::type
    { return C::emplace_back(std::forward<T>(val)); }
};

template <typename OUTCONT, typename INCONT, typename OP>
OUTCONT transform(const INCONT& input, OP&& op)
{
    using namespace SOA::Utils;
    //using namespace std;
    OUTCONT retVal;
    retVal.reserve(input.size());
    for (auto const in : input) {
        retVal.emplace_back(invoke(std::forward<OP>(op), in));
    }
    return retVal;
}

TEST(SOAZipTest, Olli)
{
    using SOAInt = SOA::Container<std::vector, Moo::s_moo>;
    using SOATwoInt = SOA::Container<std::vector, Moo::s_twoint>;
    using ZI = Container<SOAInt>;
    using ZTI = Container<SOATwoInt>;

    ZI c;
    int i = 42;
    const int& ci = i;
    c.push_back(i);
    c.push_back(ci);
    c.push_back(std::move(i));
    const ZI& cc = c;
    c.push_back(c.front().moo());
    c.push_back(cc.front().moo());
    c.push_back(std::move(c.front().moo()));

    auto tc = transform<ZI>(c, [] (ZI::const_reference i) { return std::make_tuple(i.moo()); });
    auto ttc = transform<ZTI>(c, [] (ZI::const_reference i) { return Moo::twoint{i.moo(), i.moo()}; });
    auto vc = c.view<Moo::f_moo>();
    auto ttvc = transform<ZTI>(vc, [] (decltype(vc)::reference i) { return Moo::twoint{i.moo(), i.moo()}; });
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
