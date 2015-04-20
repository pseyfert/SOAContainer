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
	assert(it == c.begin());
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
	// check if they're all the same
	assert(oldcap == static_cast<decltype(oldcap)>(
		    std::count_if(std::begin(c), std::end(c),
			[] (decltype(c)::const_reference_type obj) {
			return (obj == std::make_tuple(3.14, 42, 17)); })));
	// check the other variants of comparison operators
	assert(oldcap == static_cast<decltype(oldcap)>(
		    std::count_if(std::begin(c), std::end(c),
			[] (decltype(c)::const_reference_type obj) {
			return (std::make_tuple(3.14, 42, 17) == obj); })));
	// check if they're all >=
	assert(oldcap == static_cast<decltype(oldcap)>(
		    std::count_if(std::begin(c), std::end(c),
			[] (decltype(c)::const_reference_type obj) {
			return (obj >= std::make_tuple(3.14, 42, 17)); })));
	// check the other variants of comparison operators
	assert(oldcap == static_cast<decltype(oldcap)>(
		    std::count_if(std::begin(c), std::end(c),
			[] (decltype(c)::const_reference_type obj) {
			return (std::make_tuple(3.14, 42, 17) >= obj); })));
	// check if they're all <=
	assert(oldcap == static_cast<decltype(oldcap)>(
		    std::count_if(std::begin(c), std::end(c),
			[] (decltype(c)::const_reference_type obj) {
			return (obj <= std::make_tuple(3.14, 42, 17)); })));
	// check the other variants of comparison operators
	assert(oldcap == static_cast<decltype(oldcap)>(
		    std::count_if(std::begin(c), std::end(c),
			[] (decltype(c)::const_reference_type obj) {
			return (std::make_tuple(3.14, 42, 17) <= obj); })));
	// check if none are <
	assert(0 == static_cast<decltype(oldcap)>(
		    std::count_if(std::begin(c), std::end(c),
			[] (decltype(c)::const_reference_type obj) {
			return (obj < std::make_tuple(3.14, 42, 17)); })));
	// check the other variants of comparison operators
	assert(0 == static_cast<decltype(oldcap)>(
		    std::count_if(std::begin(c), std::end(c),
			[] (decltype(c)::const_reference_type obj) {
			return (std::make_tuple(3.14, 42, 17) < obj); })));
	// check if none are >
	assert(0 == static_cast<decltype(oldcap)>(
		    std::count_if(std::begin(c), std::end(c),
			[] (decltype(c)::const_reference_type obj) {
			return (obj > std::make_tuple(3.14, 42, 17)); })));
	// check the other variants of comparison operators
	assert(0 == static_cast<decltype(oldcap)>(
		    std::count_if(std::begin(c), std::end(c),
			[] (decltype(c)::const_reference_type obj) {
			return (std::make_tuple(3.14, 42, 17) > obj); })));
    }
    {
	// test insert(pos, first, last), erase(pos) and erase(first, last)
	// by comparing to an array-of-structures in a std::vector
	typedef std::size_t size_type;
	c.clear();
	assert(c.empty());
	std::tuple<double, int, int> val(3.14, 0, 63);
	std::vector<std::tuple<double, int, int> > temp;
	temp.reserve(64);
	for (int i = 0; i < 64; ++i) {
	    std::get<1>(val) = i;
	    std::get<2>(val) = 63 - i;
	    temp.push_back(val);
	}
	auto it = c.insert(c.begin(), temp.cbegin(), temp.cend());
	assert(c.begin() == it);
	assert(64 == c.size());
	assert(temp.size() == std::inner_product(
		    c.begin(), c.end(), temp.begin(), size_type(0),
		    [] (size_type a, size_type b) { return a + b; },
		    [] (const decltype(val)& a, const decltype(val)& b) {
		        return size_type(a == b); }));
	// erase(pos)
	auto jt = temp.erase(temp.begin() + 3);
	assert(temp.begin() + 3 == jt);
	auto kt = c.erase(c.begin() + 3);
	assert(c.begin() + 3 == kt);
	assert(temp.size() == std::inner_product(
		    c.begin(), c.end(), temp.begin(), size_type(0),
		    [] (size_type a, size_type b) { return a + b; },
		    [] (const decltype(val)& a, const decltype(val)& b) {
		        return size_type(a == b); }));
	// erase(first, last)
	auto lt = temp.erase(temp.begin() + 5, temp.begin() + 10);
	assert(temp.begin() + 5 == lt);
	auto mt = c.erase(c.begin() + 5, c.begin() + 10);
	assert(c.begin() + 5 == mt);
	assert(temp.size() == std::inner_product(
		    c.begin(), c.end(), temp.begin(), size_type(0),
		    [] (size_type a, size_type b) { return a + b; },
		    [] (const decltype(val)& a, const decltype(val)& b) {
		        return size_type(a == b); }));
    }
    {
	// test assign(count, val)
	c.assign(42, std::make_tuple(3.14, 0, -1));
	assert(42u == c.size());
	assert(c.size() == std::size_t(std::count(std::begin(c), std::end(c),
			std::make_tuple(3.14, 0, -1))));
	// assign(first, last) is just a frontend for clear(); insert(front,
	// end); - therefore, no test here
    }
    {
	// test emplace, emplace_back, resize
	c.clear();
	c.emplace_back(2.79, 42, 17);
	assert(1 == c.size());
	assert(c.front() == std::make_tuple(2.79, 42, 17));
	auto it = c.emplace(c.begin(), 2.79, 17, 42);
	assert(2 == c.size());
	assert(c.begin() == it);
	assert(c.front() == std::make_tuple(2.79, 17, 42));
	assert(c.back() == std::make_tuple(2.79, 42, 17));
	c.resize(64, std::make_tuple(3.14, 78, 17));
	assert(64 == c.size());
	assert(c.back() == std::make_tuple(3.14, 78, 17));
	c.resize(0);
	assert(c.empty());
	c.resize(32);
	assert(32 == c.size());
	const std::tuple<double, int, int> defaultval;
	assert(c.back() == defaultval);
    }
}

/// main program of unit test
int main()
{
    test();
    std::printf("All tests passed.\n");
    return 0;
}
