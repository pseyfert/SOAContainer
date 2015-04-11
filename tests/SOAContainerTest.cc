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
}

/// main program of unit test
int main()
{
    test();
    std::printf("All tests passed.\n");
    return 0;
}
