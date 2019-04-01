/** @file tests/SOAContainerVectorSimple.cc
 *
 * @brief some very basic SOA::Container tests, based on std::vector
 *
 * @author Manuel Schiller <Manuel.Schiller@cern.ch>
 * @date 2015-04-11
 *
 * For copyright and license information, see the end of the file.
 */

#include <tuple>
#include <vector>
#include <numeric>
#include <algorithm>

#include "gtest/gtest.h"
#include "SOAContainer.h"

/// unit test Container class
TEST(SOAContainerVectorSimple, IteratorsSizeEmpty)
{
    SOA::Container<std::vector, SOA::PrintableNullSkin, double, int, int> c;
    EXPECT_EQ(SOA::Utils::is_view<decltype(c)>::value, true);
    EXPECT_EQ(SOA::Utils::is_container<decltype(c)>::value, true);
    const SOA::Container<std::vector, SOA::PrintableNullSkin, double, int,
                         int>& cc = c;
    // check basic properties
    EXPECT_TRUE(c.empty());
    EXPECT_EQ(0u, c.size());
    c.clear();
    EXPECT_LE(1u, c.max_size());
    EXPECT_LE(0u, c.capacity());
    // reserve space
    c.reserve(64);
    EXPECT_LE(64u, c.capacity());
    EXPECT_LE(c.capacity(), c.max_size());
    // check iterators
    EXPECT_FALSE(c.begin());
    EXPECT_EQ(c.begin(), c.end());
    EXPECT_EQ(cc.begin(), cc.end());
    EXPECT_EQ(c.begin(), cc.begin());
    EXPECT_LE(c.begin(), c.end());
    EXPECT_LE(cc.begin(), cc.end());
    EXPECT_LE(c.begin(), cc.begin());
    EXPECT_GE(c.begin(), c.end());
    EXPECT_GE(cc.begin(), cc.end());
    // check reverse iterators
    EXPECT_GE(c.rbegin(), cc.rbegin());
    EXPECT_EQ(c.rbegin(), c.rend());
    EXPECT_EQ(cc.rbegin(), cc.rend());
    EXPECT_EQ(c.rbegin(), cc.rbegin());
    EXPECT_LE(c.rbegin(), c.rend());
    EXPECT_LE(cc.rbegin(), cc.rend());
    EXPECT_LE(c.rbegin(), cc.rbegin());
    EXPECT_GE(c.rbegin(), c.rend());
    EXPECT_GE(cc.rbegin(), cc.rend());
    EXPECT_GE(c.rbegin(), cc.rbegin());
    // test at
    EXPECT_THROW(c.at(0), std::out_of_range);
}

TEST(SOAContainerVectorSimple, BasicPushPopInsert)
{
    SOA::Container<std::vector, SOA::PrintableNullSkin, double, int, int> c;
    const SOA::Container<std::vector, SOA::PrintableNullSkin, double, int,
                         int>& cc = c;
    std::tuple<double, int, int> val(3.14, 17, 42);
    // standard push_back by const reference
    c.push_back(val);
    EXPECT_FALSE(c.empty());
    EXPECT_EQ(1u, c.size());

    // After this, Clang fails
    EXPECT_EQ(c.front(), c.back());
    EXPECT_EQ(c.end(), 1 + c.begin());
    EXPECT_EQ(c.rend(), 1 + c.rbegin());
    EXPECT_EQ(&c.front(), c.begin());
    EXPECT_EQ(&cc.front(), c.cbegin());
    const decltype(val) val2(c.front());
    EXPECT_EQ(val, val2);

    // trigger the move-variant of push_back
    c.push_back(std::make_tuple(2.79, 42, 17));

    EXPECT_EQ(2u, c.size());
    EXPECT_NE(c.front(), c.back());
    EXPECT_EQ(c.end(), 2 + c.begin());
    EXPECT_EQ(c.rend(), 2 + c.rbegin());
    // test pop_back
    c.pop_back();
    EXPECT_EQ(1u, c.size());
    // start testing plain and simple insert
    std::tuple<double, int, int> val3(2.79, 42, 17);
    auto it = c.insert(c.begin(), val3);
    EXPECT_EQ(2u, c.size());
    EXPECT_EQ(it, c.begin());
    const decltype(val) val4(c.front()), val5(c.back());
    EXPECT_EQ(val3, val4);
    EXPECT_EQ(val, val5);
    c.insert(1 + c.cbegin(), std::make_tuple(2.79, 42, 17));
    EXPECT_EQ(3u, c.size());
    const decltype(val) val6(c[0]), val7(c[1]);
    EXPECT_EQ(val3, val6);
    EXPECT_EQ(val3, val7);

    EXPECT_FALSE(c.empty());
    auto oldcap = c.capacity();
    EXPECT_GT(oldcap, 0u);
    c.clear();
    EXPECT_TRUE(c.empty());
    EXPECT_EQ(oldcap, c.capacity()); // Used to be a bug!
    c.insert(c.begin(), oldcap, std::make_tuple(3.14, 42, 17));
    EXPECT_EQ(oldcap, c.size());
    // check if they're all the same
    EXPECT_EQ(oldcap,
              static_cast<decltype(oldcap)>(std::count_if(
                      std::begin(c), std::end(c),
                      [](decltype(c)::const_reference obj) {
                          return (obj == std::make_tuple(3.14, 42, 17));
                      })));
    // check the other variants of comparison operators
    EXPECT_EQ(oldcap,
              static_cast<decltype(oldcap)>(std::count_if(
                      std::begin(c), std::end(c),
                      [](decltype(c)::const_reference obj) {
                          return (std::make_tuple(3.14, 42, 17) == obj);
                      })));
    // check if they're all >=
    EXPECT_EQ(oldcap,
              static_cast<decltype(oldcap)>(std::count_if(
                      std::begin(c), std::end(c),
                      [](decltype(c)::const_reference obj) {
                          return (obj >= std::make_tuple(3.14, 42, 17));
                      })));
    // check the other variants of comparison operators
    EXPECT_EQ(oldcap,
              static_cast<decltype(oldcap)>(std::count_if(
                      std::begin(c), std::end(c),
                      [](decltype(c)::const_reference obj) {
                          return (std::make_tuple(3.14, 42, 17) >= obj);
                      })));
    // check if they're all <=
    EXPECT_EQ(oldcap,
              static_cast<decltype(oldcap)>(std::count_if(
                      std::begin(c), std::end(c),
                      [](decltype(c)::const_reference obj) {
                          return (obj <= std::make_tuple(3.14, 42, 17));
                      })));
    // check the other variants of comparison operators
    EXPECT_EQ(oldcap,
              static_cast<decltype(oldcap)>(std::count_if(
                      std::begin(c), std::end(c),
                      [](decltype(c)::const_reference obj) {
                          return (std::make_tuple(3.14, 42, 17) <= obj);
                      })));
    // check if none are <
    EXPECT_EQ(0u, static_cast<decltype(oldcap)>(std::count_if(
                          std::begin(c), std::end(c),
                          [](decltype(c)::const_reference obj) {
                              return (obj < std::make_tuple(3.14, 42, 17));
                          })));
    // check the other variants of comparison operators
    EXPECT_EQ(0u, static_cast<decltype(oldcap)>(std::count_if(
                          std::begin(c), std::end(c),
                          [](decltype(c)::const_reference obj) {
                              return (std::make_tuple(3.14, 42, 17) < obj);
                          })));
    // check if none are >
    EXPECT_EQ(0u, static_cast<decltype(oldcap)>(std::count_if(
                          std::begin(c), std::end(c),
                          [](decltype(c)::const_reference obj) {
                              return (obj > std::make_tuple(3.14, 42, 17));
                          })));
    // check the other variants of comparison operators
    EXPECT_EQ(0u, static_cast<decltype(oldcap)>(std::count_if(
                          std::begin(c), std::end(c),
                          [](decltype(c)::const_reference obj) {
                              return (std::make_tuple(3.14, 42, 17) > obj);
                          })));
}

TEST(SOAContainerVectorSimple, WithSTLAlgorithms)
{
    SOA::Container<std::vector, SOA::PrintableNullSkin, double, int, int> c;
    const SOA::Container<std::vector, SOA::PrintableNullSkin, double, int,
                         int>& cc = c;
    // test insert(pos, first, last), erase(pos) and erase(first, last)
    // by comparing to an array-of-structures in a std::vector
    typedef std::size_t size_type;
    EXPECT_TRUE(c.empty());
    std::tuple<double, int, int> val(3.14, 0, 63);
    std::vector<std::tuple<double, int, int>> temp;
    temp.reserve(64);
    for (int i = 0; i < 64; ++i) {
        std::get<1>(val) = i;
        std::get<2>(val) = 63 - i;
        temp.push_back(val);
    }
    auto it = c.insert(c.begin(), temp.cbegin(), temp.cend());
    EXPECT_EQ(c.begin(), it);
    EXPECT_EQ(64u, c.size());
    EXPECT_EQ(c.size(), temp.size());
    EXPECT_EQ(temp.size(),
              std::inner_product(
                      c.begin(), c.end(), temp.begin(), size_type(0),
                      [](size_type a, size_type b) { return a + b; },
                      [](const decltype(val)& a, const decltype(val)& b) {
                          return size_type(a == b);
                      }));

    // erase(pos)
    auto jt = temp.erase(temp.begin() + 3);
    EXPECT_EQ(temp.begin() + 3, jt);
    auto kt = c.erase(c.begin() + 3);
    EXPECT_EQ(c.begin() + 3, kt);
    EXPECT_EQ(c.size(), temp.size());
    EXPECT_EQ(temp.size(),
              std::inner_product(
                      c.begin(), c.end(), temp.begin(), size_type(0),
                      [](size_type a, size_type b) { return a + b; },
                      [](const decltype(val)& a, const decltype(val)& b) {
                          return size_type(a == b);
                      }));

    // erase(first, last)
    auto lt = temp.erase(temp.begin() + 5, temp.begin() + 10);
    EXPECT_EQ(temp.begin() + 5, lt);
    auto mt = c.erase(c.begin() + 5, c.begin() + 10);
    EXPECT_EQ(c.begin() + 5, mt);
    EXPECT_EQ(c.size(), temp.size());
    EXPECT_EQ(temp.size(),
              std::inner_product(
                      c.begin(), c.end(), temp.begin(), size_type(0),
                      [](size_type a, size_type b) { return a + b; },
                      [](const decltype(val)& a, const decltype(val)& b) {
                          return size_type(a == b);
                      }));

    // test sort (and swap)
    EXPECT_TRUE(std::is_sorted(c.begin(), c.end(),
                               [](decltype(c)::value_const_reference a,
                                  decltype(c)::value_const_reference b) {
                                   return a.get<1>() < b.get<1>();
                               }));

    std::sort(c.begin(), c.end(),
              [](decltype(c)::value_const_reference a,
                 decltype(c)::value_const_reference b) {
                  return a.get<1>() > b.get<1>();
              });

    std::sort(temp.begin(), temp.end(),
              [](const decltype(temp)::value_type& a,
                 const decltype(temp)::value_type& b) {
                  return std::get<1>(a) > std::get<1>(b);
              });

    EXPECT_TRUE(std::is_sorted(c.begin(), c.end(),
                               [](decltype(c)::value_const_reference a,
                                  decltype(c)::value_const_reference b) {
                                   return a.get<1>() > b.get<1>();
                               }));
    EXPECT_TRUE(std::is_sorted(temp.begin(), temp.end(),
                               [](const decltype(temp)::value_type& a,
                                  const decltype(temp)::value_type& b) {
                                   return std::get<1>(a) > std::get<1>(b);
                               }));

    EXPECT_EQ(c.size(), temp.size());
    EXPECT_EQ(temp.size(),
              std::inner_product(
                      c.begin(), c.end(), temp.begin(), size_type(0),
                      [](size_type a, size_type b) { return a + b; },
                      [](const decltype(val)& a, const decltype(val)& b) {
                          return size_type(a == b);
                      }));

    // test the begin<fieldno> and end<fieldno> calls
    EXPECT_EQ(&(*c.begin<0>()), &((*c.begin()).get<0>()));
    EXPECT_EQ(&(*cc.begin<0>()), &((*cc.begin()).get<0>()));
    EXPECT_EQ(&(*c.cbegin<0>()), &((*c.cbegin()).get<0>()));

    EXPECT_EQ(&(*c.end<0>()), &((*c.end()).get<0>()));
    EXPECT_EQ(&(*cc.end<0>()), &((*cc.end()).get<0>()));
    EXPECT_EQ(&(*c.cend<0>()), &((*c.cend()).get<0>()));

    // test (some of) the rbegin<fieldno> and rend<fieldno> calls
    EXPECT_EQ(&(*c.rbegin<0>()), &((*c.rbegin()).get<0>()));
    EXPECT_EQ(&(*c.rend<0>()), &((*c.rend()).get<0>()));

    // test the begin<fieldtag> and end<fieldtag> calls
    EXPECT_EQ(&(*c.begin<double>()), &((*c.begin()).get<double>()));
    EXPECT_EQ(&(*cc.begin<double>()), &((*cc.begin()).get<double>()));
    EXPECT_EQ(&(*c.cbegin<double>()), &((*c.cbegin()).get<double>()));
    EXPECT_EQ(&(*c.end<double>()), &((*c.end()).get<double>()));
    EXPECT_EQ(&(*cc.end<double>()), &((*cc.end()).get<double>()));
    EXPECT_EQ(&(*c.cend<double>()), &((*c.cend()).get<double>()));

    // test (some of) the rbegin<fieldno> and rend<fieldno> calls
    EXPECT_EQ(&(*c.rbegin<double>()), &((*c.rbegin()).get<double>()));
    EXPECT_EQ(&(*c.rend<double>()), &((*c.rend()).get<double>()));

    // rudimentary tests of comparison of containers
    decltype(c) d;
    decltype(temp) temp2;
    EXPECT_EQ(c, c);
    EXPECT_EQ(temp, temp);
    EXPECT_NE(c, d);
    EXPECT_NE(temp, temp2);
    EXPECT_LT(d, c);
    EXPECT_LT(temp2, temp);
    EXPECT_LE(c, c);
    EXPECT_LE(temp, temp);
    EXPECT_GE(c, c);
    EXPECT_GE(temp, temp);

    {
        // test assign(count, val)
        c.assign(42, std::make_tuple(3.14, 0, -1));
        EXPECT_EQ(42u, c.size());
        EXPECT_EQ(c.size(),
                  std::size_t(std::count(std::begin(c), std::end(c),
                                         std::make_tuple(3.14, 0, -1))));
        // assign(first, last) is just a frontend for clear(); insert(front,
        // end); - therefore, no test here
    }
    {
        // test emplace, emplace_back, resize
        c.clear();
        c.emplace_back(2.79, 42, 17);
        EXPECT_EQ(1u, c.size());
        EXPECT_EQ(c.front(), std::make_tuple(2.79, 42, 17));
        auto it = c.emplace(c.begin(), 2.79, 17, 42);
        EXPECT_EQ(2u, c.size());
        EXPECT_EQ(c.begin(), it);
        EXPECT_EQ(c.front(), std::make_tuple(2.79, 17, 42));
        EXPECT_EQ(c.back(), std::make_tuple(2.79, 42, 17));
        c.resize(64, std::make_tuple(3.14, 78, 17));
        EXPECT_EQ(64u, c.size());
        EXPECT_EQ(c.back(), std::make_tuple(3.14, 78, 17));
        c.emplace_back(std::make_tuple(42., 42, 42));
        EXPECT_EQ(c.back(), std::make_tuple(42., 42, 42));
        c.emplace(c.begin(), std::make_tuple(17., 42, 42));
        EXPECT_EQ(c.front(), std::make_tuple(17., 42, 42));
        c.resize(0);
        EXPECT_TRUE(c.empty());
        c.resize(32);
        EXPECT_EQ(32u, c.size());
        const std::tuple<double, int, int> defaultval;
        EXPECT_EQ(c.back(), defaultval);
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
