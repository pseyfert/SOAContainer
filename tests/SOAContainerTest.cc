/** @file SOAContainerTest.cc
 *
 * @author Manuel Schiller <Manuel.Schiller@cern.ch>
 * @date 2015-04-11
 *
 * unit test for SOAContainer class
 */

#include <cassert>
#include <cstdio>

#include "SOAContainer.h"

/// unit test SOAContainer class
static void test()
{
    SOAContainer<std::vector, double, int, int> c;
    const SOAContainer<std::vector, double, int, int>& cc = c;
    // check basic properties
    assert(c.empty());
    assert(0 == c.size());
    c.clear();
    assert(1 <= c.max_size());
    assert(0 <= c.capacity());
    // reserve space
    c.reserve(64);
    assert(64 <= c.capacity());
    assert(c.capacity() <= c.max_size());
    // check iterators
    assert(!c.begin());
    assert(c.begin() == c.end());
    assert(cc.begin() == cc.end());
    assert(c.begin() == cc.begin());
    assert(c.begin() <= c.end());
    assert(cc.begin() <= cc.end());
    assert(c.begin() <= cc.begin());
    assert(c.begin() >= c.end());
    assert(cc.begin() >= cc.end());
    // check reverse iterators
    assert(c.rbegin() >= cc.rbegin());
    assert(c.rbegin() == c.rend());
    assert(cc.rbegin() == cc.rend());
    assert(c.rbegin() == cc.rbegin());
    assert(c.rbegin() <= c.rend());
    assert(cc.rbegin() <= cc.rend());
    assert(c.rbegin() <= cc.rbegin());
    assert(c.rbegin() >= c.rend());
    assert(cc.rbegin() >= cc.rend());
    assert(c.rbegin() >= cc.rbegin());
    // test at
    {
	bool exception = false;
	try {
	    c.at(0);
	} catch (const std::out_of_range&) {
	    exception = true;
	}
	assert(exception);
    }
    {
	std::tuple<double, int, int> val(3.14, 17, 42);
	// standard push_back by const reference
	c.push_back(val);
	assert(!c.empty());
	assert(1 == c.size());
	assert(c.front() == c.back());
	assert(c.end() == 1 + c.begin());
	assert(c.rend() == 1 + c.rbegin());
	const decltype(val) val2(c.front());
	assert(val == val2);
	// trigger the move-variant of push_back
	c.push_back(std::make_tuple(2.79, 42, 17));
	assert(2 == c.size());
	assert(c.front() != c.back());
	assert(c.end() == 2 + c.begin());
	assert(c.rend() == 2 + c.rbegin());
	// test pop_back
	c.pop_back();
	assert(1 == c.size());
	// start testing plain and simple insert
	std::tuple<double, int, int> val3(2.79, 42, 17);
	auto it = c.insert(c.begin(), val3);
	assert(2 == c.size());
	assert(it == 1 + c.begin());
	const decltype(val) val4(c.front()), val5(c.back());
	assert(val3 == val4);
	assert(val == val5);
	c.insert(1 + c.cbegin(), std::make_tuple(2.79, 42, 17));
	assert(3 == c.size());
	const decltype(val) val6(c[0]), val7(c[1]);
	assert(val3 == val6);
	assert(val3 == val7);
    }
    {
	assert(!c.empty());
	auto oldcap = c.capacity();
	assert(oldcap > 0);
	c.clear();
	assert(c.empty());
	assert(oldcap = c.capacity());
	c.insert(c.begin(), oldcap, std::make_tuple(3.14, 42, 17));
	assert(oldcap == c.size());
	assert(oldcap == static_cast<decltype(oldcap)>(
		    std::count(std::begin(c), std::end(c),
			std::make_tuple(3.14, 42, 17))));
	assert(oldcap == static_cast<decltype(oldcap)>(
		    std::count_if(std::begin(c), std::end(c),
			[] (decltype(c)::const_reference_type obj) {
			return (std::make_tuple(3.14, 42, 17) == obj); })));
    }
}

/// main program of unit test
int main()
{
    test();
    std::printf("All tests passed.\n");
    return 0;
}
