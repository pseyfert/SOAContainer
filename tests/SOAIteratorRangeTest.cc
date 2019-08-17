/** @file SOAIteratorRangeTest.cc
 *
 * @author Manuel Schiller <Manuel.Schiller@glasgow.ac.uk>
 * @date 2018-04-04
 *
 * unit test for SOA::iterator_range
 *
 * For copyright and license information, see the end of the file.
 */

#include <array>
#include <vector>

#include "SOAIteratorRange.h"

#include "gtest/gtest.h"

TEST(SOAIteratorRangeTest, Pointer) {
    using namespace SOA;
    using T = int;
    T fooarr[] = { 0, 1, 2, 3, 4, 5, 6, 7 };
    T* foo = &fooarr[0];
    auto r = make_iterator_range(&foo[1], &foo[6]);
    
    // check typedefs
    const bool iterator_cat_okay = std::is_base_of<
        typename decltype(r)::iterator_category,
                 std::random_access_iterator_tag>::value;
    EXPECT_EQ(true, iterator_cat_okay);
    const bool diff_right = std::is_same<std::ptrdiff_t,
          typename decltype(r)::difference_type>::value;
    EXPECT_EQ(true, diff_right);
    const bool size_right = std::is_same<std::size_t,
          typename decltype(r)::size_type>::value;
    EXPECT_EQ(true, size_right);
    const bool value_right = std::is_same<T,
          typename decltype(r)::value_type>::value;
    EXPECT_EQ(true, value_right);
    const bool reference_right = std::is_same<T&,
            typename decltype(r)::reference>::value;
    EXPECT_EQ(true, reference_right);
    const bool pointer_right = std::is_same<T*,
            typename decltype(r)::pointer>::value;
    EXPECT_EQ(true, pointer_right);
    const bool iterator_right = std::is_same<
        T*, typename decltype(r)::iterator>::value;
    EXPECT_EQ(true, iterator_right);
    // check class empty, size
    EXPECT_EQ(false, r.empty());
    EXPECT_EQ(5u, r.size());
    EXPECT_EQ(true, make_iterator_range(&foo[0], &foo[0]).empty());
    EXPECT_EQ(0u, make_iterator_range(&foo[0], &foo[0]).size());
    EXPECT_EQ(false, make_iterator_range(&foo[0], &foo[1]).empty());
    EXPECT_EQ(1u, make_iterator_range(&foo[0], &foo[1]).size());
    EXPECT_EQ(false, make_iterator_range(&foo[0], &foo[2]).empty());
    EXPECT_EQ(2u, make_iterator_range(&foo[0], &foo[2]).size());
    // check r's contents
    for (unsigned i = 1; i < 6; ++i) {
        EXPECT_EQ(foo[i], r[i - 1]);
    }
    EXPECT_EQ(&foo[1], r.begin());
    EXPECT_EQ(&foo[6], r.end());
    EXPECT_EQ(foo[1], r.front());
    EXPECT_EQ(foo[5], r.back());
    EXPECT_EQ(foo[5], *r.rbegin());
    EXPECT_EQ(&foo[0], &*r.rend());
    // at() near start of range
    bool except = false;
    try {
        r.at(1) = 0;
        r.at(0) = 1;
        r.at(-1) = 2;
    } catch (std::out_of_range&) {
        except = true;
    };
    EXPECT_EQ(true, except);
    EXPECT_EQ(0, foo[0]);
    EXPECT_EQ(1, foo[1]);
    EXPECT_EQ(0, foo[2]);
    // at() near end of range
    except = false;
    try {
        r.at(4) = 7;
        r.at(5) = 6;
        r.at(6) = 5;
    } catch (std::out_of_range&) {
        except = true;
    };
    EXPECT_EQ(true, except);
    EXPECT_EQ(7, foo[5]);
    EXPECT_EQ(6, foo[6]);
    EXPECT_EQ(7, foo[7]);
}

TEST(SOAIteratorRangeTest, PlainArray) {
    using namespace SOA;
    using T = int;
    T foo[] = { 0, 1, 2, 3, 4, 5, 6, 7 };
    auto r = make_iterator_range(&foo[1], &foo[6]);
    
    // check typedefs
    const bool iterator_cat_okay = std::is_base_of<
        typename decltype(r)::iterator_category,
                 std::random_access_iterator_tag>::value;
    EXPECT_EQ(true, iterator_cat_okay);
    const bool diff_right = std::is_same<std::ptrdiff_t,
          typename decltype(r)::difference_type>::value;
    EXPECT_EQ(true, diff_right);
    const bool size_right = std::is_same<std::size_t,
          typename decltype(r)::size_type>::value;
    EXPECT_EQ(true, size_right);
    const bool value_right = std::is_same<T,
          typename decltype(r)::value_type>::value;
    EXPECT_EQ(true, value_right);
    const bool reference_right = std::is_same<T&,
            typename decltype(r)::reference>::value;
    EXPECT_EQ(true, reference_right);
    const bool pointer_right = std::is_same<T*,
            typename decltype(r)::pointer>::value;
    EXPECT_EQ(true, pointer_right);
    const bool iterator_right = std::is_same<
        decltype(std::begin(foo)), typename decltype(r)::iterator>::value;
    EXPECT_EQ(true, iterator_right);
    // check class empty, size
    EXPECT_EQ(false, r.empty());
    EXPECT_EQ(5u, r.size());
    EXPECT_EQ(true, make_iterator_range(&foo[0], &foo[0]).empty());
    EXPECT_EQ(0u, make_iterator_range(&foo[0], &foo[0]).size());
    EXPECT_EQ(false, make_iterator_range(&foo[0], &foo[1]).empty());
    EXPECT_EQ(1u, make_iterator_range(&foo[0], &foo[1]).size());
    EXPECT_EQ(false, make_iterator_range(&foo[0], &foo[2]).empty());
    EXPECT_EQ(2u, make_iterator_range(&foo[0], &foo[2]).size());
    // check r's contents
    for (unsigned i = 1; i < 6; ++i) {
        EXPECT_EQ(foo[i], r[i - 1]);
    }
    EXPECT_EQ(&foo[1], r.begin());
    EXPECT_EQ(&foo[6], r.end());
    EXPECT_EQ(foo[1], r.front());
    EXPECT_EQ(foo[5], r.back());
    EXPECT_EQ(foo[5], *r.rbegin());
    EXPECT_EQ(&foo[0], &*r.rend());
    // at() near start of range
    bool except = false;
    try {
        r.at(1) = 0;
        r.at(0) = 1;
        r.at(-1) = 2;
    } catch (std::out_of_range&) {
        except = true;
    };
    EXPECT_EQ(true, except);
    EXPECT_EQ(0, foo[0]);
    EXPECT_EQ(1, foo[1]);
    EXPECT_EQ(0, foo[2]);
    // at() near end of range
    except = false;
    try {
        r.at(4) = 7;
        r.at(5) = 6;
        r.at(6) = 5;
    } catch (std::out_of_range&) {
        except = true;
    };
    EXPECT_EQ(true, except);
    EXPECT_EQ(7, foo[5]);
    EXPECT_EQ(6, foo[6]);
    EXPECT_EQ(7, foo[7]);
}

TEST(SOAIteratorRangeTest, StdArray) {
    using namespace SOA;
    using T = int;
    using C = std::array<T, 8>;
    C foo = {{ 0, 1, 2, 3, 4, 5, 6, 7 }};
    auto r = make_iterator_range(foo.begin() + 1, foo.begin() + 6);
    
    // check typedefs
    const bool iterator_cat_okay = std::is_base_of<
        typename decltype(r)::iterator_category,
                 std::random_access_iterator_tag>::value;
    EXPECT_EQ(true, iterator_cat_okay);
    const bool diff_right = std::is_same<std::ptrdiff_t,
          typename decltype(r)::difference_type>::value;
    EXPECT_EQ(true, diff_right);
    const bool size_right = std::is_same<std::size_t,
          typename decltype(r)::size_type>::value;
    EXPECT_EQ(true, size_right);
    const bool value_right = std::is_same<typename C::value_type,
          typename decltype(r)::value_type>::value;
    EXPECT_EQ(true, value_right);
    const bool reference_right = std::is_same<typename C::reference,
            typename decltype(r)::reference>::value;
    EXPECT_EQ(true, reference_right);
    const bool pointer_right = std::is_same<typename C::pointer,
            typename decltype(r)::pointer>::value;
    EXPECT_EQ(true, pointer_right);
    const bool iterator_right = std::is_same<
        decltype(std::begin(foo)), typename decltype(r)::iterator>::value;
    EXPECT_EQ(true, iterator_right);
    // check class empty, size
    EXPECT_EQ(false, r.empty());
    EXPECT_EQ(5u, r.size());
    EXPECT_EQ(true, make_iterator_range(foo.begin() + 0, foo.begin() + 0).empty());
    EXPECT_EQ(0u, make_iterator_range(foo.begin() + 0, foo.begin() + 0).size());
    EXPECT_EQ(false, make_iterator_range(foo.begin() + 0, foo.begin() + 1).empty());
    EXPECT_EQ(1u, make_iterator_range(foo.begin() + 0, foo.begin() + 1).size());
    EXPECT_EQ(false, make_iterator_range(foo.begin() + 0, foo.begin() + 2).empty());
    EXPECT_EQ(2u, make_iterator_range(foo.begin() + 0, foo.begin() + 2).size());
    // check r's contents
    for (unsigned i = 1; i < 6; ++i) {
        EXPECT_EQ(foo[i], r[i - 1]);
    }
    EXPECT_EQ(foo.begin() + 1, r.begin());
    EXPECT_EQ(foo.begin() + 6, r.end());
    EXPECT_EQ(*(foo.begin() + 1), r.front());
    EXPECT_EQ(*(foo.begin() + 5), r.back());
    EXPECT_EQ(*(foo.begin() + 5), *r.rbegin());
    EXPECT_EQ(foo.begin() + 0, &*r.rend());
    // at() near start of range
    bool except = false;
    try {
        r.at(1) = 0;
        r.at(0) = 1;
        r.at(-1) = 2;
    } catch (std::out_of_range&) {
        except = true;
    };
    EXPECT_EQ(true, except);
    EXPECT_EQ(0, *(foo.begin() + 0));
    EXPECT_EQ(1, *(foo.begin() + 1));
    EXPECT_EQ(0, *(foo.begin() + 2));
    // at() near end of range
    except = false;
    try {
        r.at(4) = 7;
        r.at(5) = 6;
        r.at(6) = 5;
    } catch (std::out_of_range&) {
        except = true;
    };
    EXPECT_EQ(true, except);
    EXPECT_EQ(7, *(foo.begin() + 5));
    EXPECT_EQ(6, *(foo.begin() + 6));
    EXPECT_EQ(7, *(foo.begin() + 7));
}

TEST(SOAIteratorRangeTest, StdVector) {
    using namespace SOA;
    using T = int;
    using C = std::vector<T>;
    {
        C emptyc;
        auto emptyr = make_iterator_range(emptyc.begin(), emptyc.end());
        EXPECT_EQ(emptyr.empty(), true);
    }
    C foo{{ 0, 1, 2, 3, 4, 5, 6, 7 }};
    auto r = make_iterator_range(foo.begin() + 1, foo.begin() + 6);
    
    // check typedefs
    const bool iterator_cat_okay = std::is_base_of<
        typename decltype(r)::iterator_category,
                 std::random_access_iterator_tag>::value;
    EXPECT_EQ(true, iterator_cat_okay);
    const bool diff_right = std::is_same<std::ptrdiff_t,
          typename decltype(r)::difference_type>::value;
    EXPECT_EQ(true, diff_right);
    const bool size_right = std::is_same<std::size_t,
          typename decltype(r)::size_type>::value;
    EXPECT_EQ(true, size_right);
    const bool value_right = std::is_same<typename C::value_type,
          typename decltype(r)::value_type>::value;
    EXPECT_EQ(true, value_right);
    const bool reference_right = std::is_same<typename C::reference,
            typename decltype(r)::reference>::value;
    EXPECT_EQ(true, reference_right);
    const bool pointer_right = std::is_same<typename C::pointer,
            typename decltype(r)::pointer>::value;
    EXPECT_EQ(true, pointer_right);
    const bool iterator_right =
            std::is_same<decltype(std::begin(foo)),
                         typename decltype(r)::iterator>::value ||
            std::is_same<typename decltype(foo)::pointer,
                         typename decltype(r)::iterator>::value;
    EXPECT_EQ(true, iterator_right);
    // check class empty, size
    EXPECT_EQ(false, r.empty());
    EXPECT_EQ(5u, r.size());
    EXPECT_EQ(true, make_iterator_range(foo.begin() + 0, foo.begin() + 0).empty());
    EXPECT_EQ(0u, make_iterator_range(foo.begin() + 0, foo.begin() + 0).size());
    EXPECT_EQ(false, make_iterator_range(foo.begin() + 0, foo.begin() + 1).empty());
    EXPECT_EQ(1u, make_iterator_range(foo.begin() + 0, foo.begin() + 1).size());
    EXPECT_EQ(false, make_iterator_range(foo.begin() + 0, foo.begin() + 2).empty());
    EXPECT_EQ(2u, make_iterator_range(foo.begin() + 0, foo.begin() + 2).size());
    // check r's contents
    for (unsigned i = 1; i < 6; ++i) {
        EXPECT_EQ(foo[i], r[i - 1]);
    }
    EXPECT_EQ(&*foo.begin() + 1, &*r.begin());
    EXPECT_EQ(&*foo.begin() + 6, &*r.end());
    EXPECT_EQ(*(foo.begin() + 1), r.front());
    EXPECT_EQ(*(foo.begin() + 5), r.back());
    EXPECT_EQ(*(foo.begin() + 5), *r.rbegin());
    EXPECT_EQ(&*(foo.begin() + 0), &*r.rend());
    // at() near start of range
    bool except = false;
    try {
        r.at(1) = 0;
        r.at(0) = 1;
        r.at(-1) = 2;
    } catch (std::out_of_range&) {
        except = true;
    };
    EXPECT_EQ(true, except);
    EXPECT_EQ(0, *(foo.begin() + 0));
    EXPECT_EQ(1, *(foo.begin() + 1));
    EXPECT_EQ(0, *(foo.begin() + 2));
    // at() near end of range
    except = false;
    try {
        r.at(4) = 7;
        r.at(5) = 6;
        r.at(6) = 5;
    } catch (std::out_of_range&) {
        except = true;
    };
    EXPECT_EQ(true, except);
    EXPECT_EQ(7, *(foo.begin() + 5));
    EXPECT_EQ(6, *(foo.begin() + 6));
    EXPECT_EQ(7, *(foo.begin() + 7));
}

TEST(SOAIteratorRangeTest, MakeIteratorRangeFromReferencesToIterators) {
    using C = std::vector<int>;
    C v({ 0, 1, 2, 3, 4, 5, 6, 7 });
    auto it = v.begin(), itEnd = v.end();
    auto& rit = it, ritEnd = itEnd;
    auto r = SOA::make_iterator_range(rit, ritEnd);
    auto kt = v.begin(), ktEnd = v.end();
    auto jt = r.begin();
    for (; ktEnd != kt; ++jt, ++kt) {
        EXPECT_EQ(*kt, *jt);
    }
}

TEST(SOAIteratorRangeTest, ConversionToConstIterRange) {
    using C = std::vector<int>;
    C v({ 0, 1, 2, 3, 4, 5, 6, 7 });
    auto r = SOA::make_iterator_range(v.begin(), v.end());
    SOA::iterator_range<typename C::const_pointer> rc(r);
    auto it = v.begin(), itEnd = v.end();
    auto jt = r.begin();
    auto kt = rc.begin();
    for (; itEnd != it; ++it, ++jt, ++kt) {
        EXPECT_EQ(*it, *jt);
        EXPECT_EQ(*it, *kt);
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
