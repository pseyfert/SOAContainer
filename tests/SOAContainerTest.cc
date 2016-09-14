/** @file SOAContainerTest.cc
 *
 * @author Manuel Schiller <Manuel.Schiller@cern.ch>
 * @date 2015-04-11
 *
 * unit test for SOAContainer class
 */

#include <numeric>
#include <array>
#include <tuple>
#include <cmath>

#include "SOAContainer.h"
#include "gtest/gtest.h"

/// unit test SOAContainer class
TEST (BasicTest, SimpleTests) {
    SOAContainer<std::vector, NullSkin, double, int, int> c;
    const SOAContainer<std::vector, NullSkin, double, int, int>& cc = c;
    // check basic properties
    EXPECT_TRUE(c.empty());
    EXPECT_EQ(0, c.size());
    c.clear();
    EXPECT_LE(1, c.max_size());
    EXPECT_LE(0, c.capacity());
    // reserve space
    c.reserve(64);
    EXPECT_LE(64, c.capacity());
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

TEST (BasicTest, More)  {
    SOAContainer<std::vector, NullSkin, double, int, int> c;
    const SOAContainer<std::vector, NullSkin, double, int, int>& cc = c;
    std::tuple<double, int, int> val(3.14, 17, 42);
    // standard push_back by const reference
    c.push_back(val);
    EXPECT_FALSE(c.empty());
    EXPECT_EQ(1, c.size());
    
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

    
    EXPECT_EQ(2, c.size());
    EXPECT_NE(c.front(), c.back());
    EXPECT_EQ(c.end(), 2 + c.begin());
    EXPECT_EQ(c.rend(), 2 + c.rbegin());
    // test pop_back
    c.pop_back();
    EXPECT_EQ(1, c.size());
    // start testing plain and simple insert
    std::tuple<double, int, int> val3(2.79, 42, 17);
    auto it = c.insert(c.begin(), val3);
    EXPECT_EQ(2, c.size());
    EXPECT_EQ(it, c.begin());
    const decltype(val) val4(c.front()), val5(c.back());
    EXPECT_EQ(val3, val4);
    EXPECT_EQ(val, val5);
    c.insert(1 + c.cbegin(), std::make_tuple(2.79, 42, 17));
    EXPECT_EQ(3, c.size());
    const decltype(val) val6(c[0]), val7(c[1]);
    EXPECT_EQ(val3, val6);
    EXPECT_EQ(val3, val7);

    EXPECT_FALSE(c.empty());
    auto oldcap = c.capacity();
    EXPECT_GT(oldcap, 0);
    c.clear();
    EXPECT_TRUE(c.empty());
    EXPECT_EQ(oldcap, c.capacity()); // Used to be a bug!
    c.insert(c.begin(), oldcap, std::make_tuple(3.14, 42, 17));
    EXPECT_EQ(oldcap, c.size());
    // check if they're all the same
    EXPECT_EQ(oldcap, static_cast<decltype(oldcap)>(
                std::count_if(std::begin(c), std::end(c),
                    [] (decltype(c)::const_reference obj) {
                    return (obj == std::make_tuple(3.14, 42, 17)); })));
    // check the other variants of comparison operators
    EXPECT_EQ(oldcap, static_cast<decltype(oldcap)>(
                std::count_if(std::begin(c), std::end(c),
                    [] (decltype(c)::const_reference obj) {
                    return (std::make_tuple(3.14, 42, 17) == obj); })));
    // check if they're all >=
    EXPECT_EQ(oldcap, static_cast<decltype(oldcap)>(
                std::count_if(std::begin(c), std::end(c),
                    [] (decltype(c)::const_reference obj) {
                    return (obj >= std::make_tuple(3.14, 42, 17)); })));
    // check the other variants of comparison operators
    EXPECT_EQ(oldcap, static_cast<decltype(oldcap)>(
                std::count_if(std::begin(c), std::end(c),
                    [] (decltype(c)::const_reference obj) {
                    return (std::make_tuple(3.14, 42, 17) >= obj); })));
    // check if they're all <=
    EXPECT_EQ(oldcap, static_cast<decltype(oldcap)>(
                std::count_if(std::begin(c), std::end(c),
                    [] (decltype(c)::const_reference obj) {
                    return (obj <= std::make_tuple(3.14, 42, 17)); })));
    // check the other variants of comparison operators
    EXPECT_EQ(oldcap, static_cast<decltype(oldcap)>(
                std::count_if(std::begin(c), std::end(c),
                    [] (decltype(c)::const_reference obj) {
                    return (std::make_tuple(3.14, 42, 17) <= obj); })));
    // check if none are <
    EXPECT_EQ(0, static_cast<decltype(oldcap)>(
                std::count_if(std::begin(c), std::end(c),
                    [] (decltype(c)::const_reference obj) {
                    return (obj < std::make_tuple(3.14, 42, 17)); })));
    // check the other variants of comparison operators
    EXPECT_EQ(0, static_cast<decltype(oldcap)>(
                std::count_if(std::begin(c), std::end(c),
                    [] (decltype(c)::const_reference obj) {
                    return (std::make_tuple(3.14, 42, 17) < obj); })));
    // check if none are >
    EXPECT_EQ(0, static_cast<decltype(oldcap)>(
                std::count_if(std::begin(c), std::end(c),
                    [] (decltype(c)::const_reference obj) {
                    return (obj > std::make_tuple(3.14, 42, 17)); })));
    // check the other variants of comparison operators
    EXPECT_EQ(0, static_cast<decltype(oldcap)>(
                std::count_if(std::begin(c), std::end(c),
                    [] (decltype(c)::const_reference obj) {
                    return (std::make_tuple(3.14, 42, 17) > obj); })));
};
 
 
 TEST (BasicTest, EvenMore) {
   SOAContainer<std::vector, NullSkin, double, int, int> c;
   const SOAContainer<std::vector, NullSkin, double, int, int>& cc = c;
    // test insert(pos, first, last), erase(pos) and erase(first, last)
    // by comparing to an array-of-structures in a std::vector
    typedef std::size_t size_type;
    EXPECT_TRUE(c.empty());
    std::tuple<double, int, int> val(3.14, 0, 63);
    std::vector<std::tuple<double, int, int> > temp;
    temp.reserve(64);
    for (int i = 0; i < 64; ++i) {
        std::get<1>(val) = i;
        std::get<2>(val) = 63 - i;
        temp.push_back(val);
    }
    auto it = c.insert(c.begin(), temp.cbegin(), temp.cend());
    EXPECT_EQ(c.begin(), it);
    EXPECT_EQ(64, c.size());
    EXPECT_EQ(c.size(), temp.size());
    EXPECT_EQ(temp.size(), std::inner_product(
                c.begin(), c.end(), temp.begin(), size_type(0),
                [] (size_type a, size_type b) { return a + b; },
                [] (const decltype(val)& a, const decltype(val)& b) {
                    return size_type(a == b); }));

    // erase(pos)
    auto jt = temp.erase(temp.begin() + 3);
    EXPECT_EQ(temp.begin() + 3, jt);
    auto kt = c.erase(c.begin() + 3);
    EXPECT_EQ(c.begin() + 3, kt);
    EXPECT_EQ(c.size(), temp.size());
    EXPECT_EQ(temp.size(), std::inner_product(
                c.begin(), c.end(), temp.begin(), size_type(0),
                [] (size_type a, size_type b) { return a + b; },
                [] (const decltype(val)& a, const decltype(val)& b) {
                    return size_type(a == b); }));

    // erase(first, last)
    auto lt = temp.erase(temp.begin() + 5, temp.begin() + 10);
    EXPECT_EQ(temp.begin() + 5, lt);
    auto mt = c.erase(c.begin() + 5, c.begin() + 10);
    EXPECT_EQ(c.begin() + 5, mt);
    EXPECT_EQ(c.size(), temp.size());
    EXPECT_EQ(temp.size(), std::inner_product(
                c.begin(), c.end(), temp.begin(), size_type(0),
                [] (size_type a, size_type b) { return a + b; },
                [] (const decltype(val)& a, const decltype(val)& b) {
                    return size_type(a == b); }));

    // test sort (and swap)
    EXPECT_TRUE(std::is_sorted(c.begin(), c.end(),
            [] (decltype(c)::value_const_reference a,
                decltype(c)::value_const_reference b)
            { return a.get<1>() < b.get<1>(); }));
    std::sort(c.begin(), c.end(),
            [] (decltype(c)::value_const_reference a,
                decltype(c)::value_const_reference b)
            { return a.get<1>() > b.get<1>(); });
    std::sort(temp.begin(), temp.end(),
            [] (const decltype(temp)::value_type& a,
        const decltype(temp)::value_type& b)
            { return std::get<1>(a) > std::get<1>(b); });
    EXPECT_TRUE(std::is_sorted(c.begin(), c.end(),
            [] (decltype(c)::value_const_reference a,
                decltype(c)::value_const_reference b)
            { return a.get<1>() > b.get<1>(); }));
    EXPECT_TRUE(std::is_sorted(temp.begin(), temp.end(),
            [] (const decltype(temp)::value_type& a,
        const decltype(temp)::value_type& b)
            { return std::get<1>(a) > std::get<1>(b); }));
    EXPECT_EQ(c.size(), temp.size());
    EXPECT_EQ(temp.size(), std::inner_product(
                c.begin(), c.end(), temp.begin(), size_type(0),
                [] (size_type a, size_type b) { return a + b; },
                [] (const decltype(val)& a, const decltype(val)& b) {
                    return size_type(a == b); }));

    // test the begin<fieldno> and end<fieldno> calls
    EXPECT_EQ(&(*c.begin<0>()), &(c.begin()->get<0>()));
    EXPECT_EQ(&(*cc.begin<0>()), &(cc.begin()->get<0>()));
    EXPECT_EQ(&(*c.cbegin<0>()), &(c.cbegin()->get<0>()));
    EXPECT_EQ(&(*c.end<0>()), &(c.end()->get<0>()));
    EXPECT_EQ(&(*cc.end<0>()), &(cc.end()->get<0>()));
    EXPECT_EQ(&(*c.cend<0>()), &(c.cend()->get<0>()));
    // test (some of) the rbegin<fieldno> and rend<fieldno> calls
    EXPECT_EQ(&(*c.rbegin<0>()), &(c.rbegin()->get<0>()));
    EXPECT_EQ(&(*c.rend<0>()), &(c.rend()->get<0>()));
    // test the begin<fieldtag> and end<fieldtag> calls
    EXPECT_EQ(&(*c.begin<double>()), &(c.begin()->get<double>()));
    EXPECT_EQ(&(*cc.begin<double>()), &(cc.begin()->get<double>()));
    EXPECT_EQ(&(*c.cbegin<double>()), &(c.cbegin()->get<double>()));
    EXPECT_EQ(&(*c.end<double>()), &(c.end()->get<double>()));
    EXPECT_EQ(&(*cc.end<double>()), &(cc.end()->get<double>()));
    EXPECT_EQ(&(*c.cend<double>()), &(c.cend()->get<double>()));
    // test (some of) the rbegin<fieldno> and rend<fieldno> calls
    EXPECT_EQ(&(*c.rbegin<double>()), &(c.rbegin()->get<double>()));
    EXPECT_EQ(&(*c.rend<double>()), &(c.rend()->get<double>()));

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
        EXPECT_EQ(c.size(), std::size_t(std::count(std::begin(c), std::end(c),
                        std::make_tuple(3.14, 0, -1))));
        // assign(first, last) is just a frontend for clear(); insert(front,
        // end); - therefore, no test here
    }
    {
        // test emplace, emplace_back, resize
        c.clear();
        c.emplace_back(2.79, 42, 17);
        EXPECT_EQ(1, c.size());
        EXPECT_EQ(c.front(), std::make_tuple(2.79, 42, 17));
        auto it = c.emplace(c.begin(), 2.79, 17, 42);
        EXPECT_EQ(2, c.size());
        EXPECT_EQ(c.begin(), it);
        EXPECT_EQ(c.front(), std::make_tuple(2.79, 17, 42));
        EXPECT_EQ(c.back(), std::make_tuple(2.79, 42, 17));
        c.resize(64, std::make_tuple(3.14, 78, 17));
        EXPECT_EQ(64, c.size());
        EXPECT_EQ(c.back(), std::make_tuple(3.14, 78, 17));
	c.emplace_back(std::make_tuple(42., 42, 42));
        EXPECT_EQ(c.back(), std::make_tuple(42., 42, 42));
	c.emplace(c.begin(), std::make_tuple(17., 42, 42));
        EXPECT_EQ(c.front(), std::make_tuple(17., 42, 42));
        c.resize(0);
        EXPECT_TRUE(c.empty());
        c.resize(32);
        EXPECT_EQ(32, c.size());
        const std::tuple<double, int, int> defaultval;
        EXPECT_EQ(c.back(), defaultval);
    }
}

namespace HitNamespace {
    namespace Fields {
        typedef struct : public SOATypelist::wrap_type<float> {} xAtYEq0;
        typedef struct : public SOATypelist::wrap_type<float> {} zAtYEq0;
        typedef struct : public SOATypelist::wrap_type<float> {} dxdy;
        typedef struct : public SOATypelist::wrap_type<float> {} dzdy;
        typedef struct : public SOATypelist::wrap_type<float> {} x;
        typedef struct : public SOATypelist::wrap_type<float> {} z;
        typedef struct : public SOATypelist::wrap_type<float> {} y;
    }

    template <typename NAKEDPROXY>
    class HitSkin : public NullSkin<NAKEDPROXY> {
        public:
            template <typename... ARGS>
            HitSkin(ARGS&&... args) : NullSkin<NAKEDPROXY>(std::forward<ARGS>(args)...) { }

            auto xAtYEq0() const noexcept -> decltype(this->template get<Fields::xAtYEq0>())
            { return this->template get<Fields::xAtYEq0>(); }
            auto zAtYEq0() const noexcept -> decltype(this->template get<Fields::zAtYEq0>())
            { return this->template get<Fields::zAtYEq0>(); }
            auto x() const noexcept -> decltype(this->template get<Fields::x>())
            { return this->template get<Fields::x>(); }
            auto y() const noexcept -> decltype(this->template get<Fields::y>())
            { return this->template get<Fields::y>(); }
            auto z() const noexcept -> decltype(this->template get<Fields::z>())
            { return this->template get<Fields::z>(); }
            auto dxdy() const noexcept -> decltype(this->template get<Fields::dxdy>())
            { return this->template get<Fields::dxdy>(); }
            auto dzdy() const noexcept -> decltype(this->template get<Fields::dzdy>())
            { return this->template get<Fields::dzdy>(); }

            void setX(float x) noexcept
            { this->template get<Fields::x>() = x; }
            void setY(float y) noexcept
            { this->template get<Fields::y>() = y; }
            void setZ(float z) noexcept
            { this->template get<Fields::z>() = z; }

            auto x(float y) const noexcept -> decltype(this->xAtYEq0() + this->dxdy() * y)
            { return xAtYEq0() + dxdy() * y; }
            auto z(float y) const noexcept -> decltype(this->zAtYEq0() + this->dzdy() * y)
            { return zAtYEq0() + dzdy() * y; }

            float y(float y0, float ySl) const noexcept
            { return (y0 + zAtYEq0() * ySl) / (1 - dzdy() * ySl); }

            float updateHit(float y0, float ySl) noexcept
            {
                setY(y(y0, ySl));
                setZ(z(y())), setX(x(y()));
                return y();
            }
    };

    using namespace Fields;
    typedef SOAContainer<std::vector, HitSkin, xAtYEq0, zAtYEq0, dxdy, dzdy, x, z, y> Hits;
    typedef typename Hits::reference Hit;
}

static void updateHits(HitNamespace::Hits& hits, float y0, float ySl) __attribute__((noinline));
static void updateHits(HitNamespace::Hits& hits, float y0, float ySl)
{
    using namespace HitNamespace;
    for (auto hit: hits) hit.updateHit(y0, ySl);
}

static void updateHits_v(HitNamespace::Hits& hits, float y0, float ySl) __attribute__((noinline));
static void updateHits_v(HitNamespace::Hits& hits, float y0, float ySl)
{
    using namespace HitNamespace;
    // give compiler a chance to not run out of registers, so it can
    // autovectorise
    for (auto hit: hits) hit.setY(hit.y(y0, ySl));
    for (auto hit: hits) { const auto y = hit.y(); hit.setZ(hit.z(y)), hit.setX(hit.x(y)); }
}

class AOSHit {
    private:
        float m_xAtYEq0;
        float m_zAtYEq0;
        float m_dxdy;
        float m_dzdy;
        float m_x;
        float m_z;
        float m_y;

    public:
        AOSHit(float xAtYEq0, float zAtYEq0, float dxdy, float dzdy, float x, float z, float y) :
            m_xAtYEq0(xAtYEq0), m_zAtYEq0(zAtYEq0),
            m_dxdy(dxdy), m_dzdy(dzdy), m_x(x), m_z(z), m_y(y)
        { }

        float xAtYEq0() const noexcept { return m_xAtYEq0; }
        float zAtYEq0() const noexcept { return m_zAtYEq0; }
        float dxdy() const noexcept { return m_dxdy; }
        float dzdy() const noexcept { return m_dzdy; }
        float x() const noexcept { return m_x; }
        float z() const noexcept { return m_z; }
        float y() const noexcept { return m_y; }
        float x(float y) const noexcept { return xAtYEq0() + dxdy() * y; }
        float z(float y) const noexcept { return zAtYEq0() + dzdy() * y; }
        float y(float y0, float ySl) const noexcept
        { return (y0 + zAtYEq0() * ySl) / (1 - dzdy() * ySl); }

        void setX(float x) noexcept { m_x = x; }
        void setY(float y) noexcept { m_y = y; }
        void setZ(float z) noexcept { m_z = z; }

        float updateHit(float y0, float ySl) noexcept
        {
            setY(y(y0, ySl));
            setZ(z(y())), setX(x(y()));
            return y();
        }
};

typedef std::vector<AOSHit> AOSHits;

static void updateHits(AOSHits& hits, float y0, float ySl) __attribute__((noinline));
static void updateHits(AOSHits& hits, float y0, float ySl)
{
    for (auto& hit: hits) hit.updateHit(y0, ySl);
}

static void updateHits_v(AOSHits& hits, float y0, float ySl) __attribute__((noinline));
static void updateHits_v(AOSHits& hits, float y0, float ySl)
{
    // give compiler a chance to not run out of registers, so it can
    // autovectorise
    for (auto& hit: hits) hit.setY(hit.y(y0, ySl));
    for (auto& hit: hits) { const auto y = hit.y(); hit.setZ(hit.z(y)), hit.setX(hit.x(y)); }
}

TEST(RealisticTest, Simple)
{
    using namespace HitNamespace;
    Hits hits;
    hits.reserve(1024);
    for (unsigned i = 0; i < 1024; ++i) {
        hits.emplace_back(0.5f * i, 8500.f, std::tan(5.f / 180.f * float(M_PI)), 3.6e-3f, 0.5f * i, 8500.f, 0.f);
    }
    for (unsigned i = 0; i < 512; ++i) updateHits(hits, 300.f, -0.01f);
    for (unsigned i = 0; i < 512; ++i) updateHits_v(hits, 300.f, -0.01f);
}

TEST(RealisticTest, Aos)
{
    using namespace HitNamespace;
    AOSHits ahits;
    ahits.reserve(1024);
    for (unsigned i = 0; i < 1024; ++i) {
        ahits.emplace_back(0.5f * i, 8500.f, std::tan(5.f / 180.f * float(M_PI)), 3.6e-3f, 0.5f * i, 8500.f, 0.f);
    }
    for (unsigned i = 0; i < 512; ++i) updateHits(ahits, 300.f, -0.01f);
    for (unsigned i = 0; i < 512; ++i) updateHits_v(ahits, 300.f, -0.01f);
}

namespace stdarraytest_fields {
    typedef std::array<unsigned, 16> Array;
    typedef SOATypelist::wrap_type<Array> f_array;

    template <typename NAKEDPROXY>
    class ContainerSkin : public NullSkin<NAKEDPROXY> {
	public:
	    typedef ContainerSkin<NAKEDPROXY> self_type;
	    typedef stdarraytest_fields::Array Array;

	    /// forward constructor to underlying type
	    template <typename... ARGS>
	    ContainerSkin(ARGS&&... args) : NullSkin<NAKEDPROXY>(
		    std::forward<ARGS>(args)...) { }


	    /// stupid constructor from an (ignored) bool
	    ContainerSkin(bool) : ContainerSkin(Array())
	    { }
    };

    typedef SOAContainer<std::vector, ContainerSkin, f_array> SOAArray;
}
TEST(RealisticTest, Proxy) {
    using namespace stdarraytest_fields;
	SOAArray a;
	a.push_back(SOAArray::value_type(true));
	a.emplace_back(SOAArray::value_type(true));
	// this won't work since we currently have no way to "dress" an
	// emplace_back with a skin (may be possible in the future)
	//a.emplace_back(true);
}
 
