/** @file SOAContainerTest.cc
 *
 * @author Manuel Schiller <Manuel.Schiller@cern.ch>
 * @date 2015-04-11
 *
 * unit test for SOAContainer class
 */

#include <cstdio>
#include <numeric>

#include "SOAContainer.h"

static unsigned ntestsfail = 0;

/// implementation of an assert-like framework (keeps working with -DNDEBUG)
#define do_test_impl(cond, condstr, line, file, func) \
        { if (!(cond)) { \
            ++ntestsfail; \
            std::printf("In routine \"%s\" (line %u in %s): " \
                    "test failed, condition\n\t\"%s\"\n", \
                    func, line, file, condstr); \
        } }
#if defined(__GNUC__)
/// test entry point (variant for gcc-like compilers with __PRETTY_FUNCTION__)
#define do_test(cond) do_test_impl((cond), #cond, \
        __LINE__, __FILE__, __PRETTY_FUNCTION__)
#else
/// test entry point (generic variant for compilers with __func__)
#define do_test(cond) do_test_impl((cond), #cond, \
        __LINE__, __FILE__, __func__)
#endif

/// unit test SOAContainer class
static void test()
{
    SOAContainer<std::vector, NullSkin, double, int, int> c;
    const SOAContainer<std::vector, NullSkin, double, int, int>& cc = c;
    // check basic properties
    do_test(c.empty());
    do_test(0 == c.size());
    c.clear();
    do_test(1 <= c.max_size());
    do_test(0 <= c.capacity());
    // reserve space
    c.reserve(64);
    do_test(64 <= c.capacity());
    do_test(c.capacity() <= c.max_size());
    // check iterators
    do_test(!c.begin());
    do_test(c.begin() == c.end());
    do_test(cc.begin() == cc.end());
    do_test(c.begin() == cc.begin());
    do_test(c.begin() <= c.end());
    do_test(cc.begin() <= cc.end());
    do_test(c.begin() <= cc.begin());
    do_test(c.begin() >= c.end());
    do_test(cc.begin() >= cc.end());
    // check reverse iterators
    do_test(c.rbegin() >= cc.rbegin());
    do_test(c.rbegin() == c.rend());
    do_test(cc.rbegin() == cc.rend());
    do_test(c.rbegin() == cc.rbegin());
    do_test(c.rbegin() <= c.rend());
    do_test(cc.rbegin() <= cc.rend());
    do_test(c.rbegin() <= cc.rbegin());
    do_test(c.rbegin() >= c.rend());
    do_test(cc.rbegin() >= cc.rend());
    do_test(c.rbegin() >= cc.rbegin());
    // test at
    {
        bool exception = false;
        try {
            c.at(0);
        } catch (const std::out_of_range&) {
            exception = true;
        }
        do_test(exception);
    }
    {
        std::tuple<double, int, int> val(3.14, 17, 42);
        // standard push_back by const reference
        c.push_back(val);
        do_test(!c.empty());
        do_test(1 == c.size());
        do_test(c.front() == c.back());
        do_test(c.end() == 1 + c.begin());
        do_test(c.rend() == 1 + c.rbegin());
        do_test(&c.front() == c.begin());
        do_test(&cc.front() == c.cbegin());
        const decltype(val) val2(c.front());
        do_test(val == val2);
        // trigger the move-variant of push_back
        c.push_back(std::make_tuple(2.79, 42, 17));
        do_test(2 == c.size());
        do_test(c.front() != c.back());
        do_test(c.end() == 2 + c.begin());
        do_test(c.rend() == 2 + c.rbegin());
        // test pop_back
        c.pop_back();
        do_test(1 == c.size());
        // start testing plain and simple insert
        std::tuple<double, int, int> val3(2.79, 42, 17);
        auto it = c.insert(c.begin(), val3);
        do_test(2 == c.size());
        do_test(it == c.begin());
        const decltype(val) val4(c.front()), val5(c.back());
        do_test(val3 == val4);
        do_test(val == val5);
        c.insert(1 + c.cbegin(), std::make_tuple(2.79, 42, 17));
        do_test(3 == c.size());
        const decltype(val) val6(c[0]), val7(c[1]);
        do_test(val3 == val6);
        do_test(val3 == val7);
    }
    {
        do_test(!c.empty());
        auto oldcap = c.capacity();
        do_test(oldcap > 0);
        c.clear();
        do_test(c.empty());
        do_test(oldcap = c.capacity());
        c.insert(c.begin(), oldcap, std::make_tuple(3.14, 42, 17));
        do_test(oldcap == c.size());
        // check if they're all the same
        do_test(oldcap == static_cast<decltype(oldcap)>(
                    std::count_if(std::begin(c), std::end(c),
                        [] (decltype(c)::const_reference obj) {
                        return (obj == std::make_tuple(3.14, 42, 17)); })));
        // check the other variants of comparison operators
        do_test(oldcap == static_cast<decltype(oldcap)>(
                    std::count_if(std::begin(c), std::end(c),
                        [] (decltype(c)::const_reference obj) {
                        return (std::make_tuple(3.14, 42, 17) == obj); })));
        // check if they're all >=
        do_test(oldcap == static_cast<decltype(oldcap)>(
                    std::count_if(std::begin(c), std::end(c),
                        [] (decltype(c)::const_reference obj) {
                        return (obj >= std::make_tuple(3.14, 42, 17)); })));
        // check the other variants of comparison operators
        do_test(oldcap == static_cast<decltype(oldcap)>(
                    std::count_if(std::begin(c), std::end(c),
                        [] (decltype(c)::const_reference obj) {
                        return (std::make_tuple(3.14, 42, 17) >= obj); })));
        // check if they're all <=
        do_test(oldcap == static_cast<decltype(oldcap)>(
                    std::count_if(std::begin(c), std::end(c),
                        [] (decltype(c)::const_reference obj) {
                        return (obj <= std::make_tuple(3.14, 42, 17)); })));
        // check the other variants of comparison operators
        do_test(oldcap == static_cast<decltype(oldcap)>(
                    std::count_if(std::begin(c), std::end(c),
                        [] (decltype(c)::const_reference obj) {
                        return (std::make_tuple(3.14, 42, 17) <= obj); })));
        // check if none are <
        do_test(0 == static_cast<decltype(oldcap)>(
                    std::count_if(std::begin(c), std::end(c),
                        [] (decltype(c)::const_reference obj) {
                        return (obj < std::make_tuple(3.14, 42, 17)); })));
        // check the other variants of comparison operators
        do_test(0 == static_cast<decltype(oldcap)>(
                    std::count_if(std::begin(c), std::end(c),
                        [] (decltype(c)::const_reference obj) {
                        return (std::make_tuple(3.14, 42, 17) < obj); })));
        // check if none are >
        do_test(0 == static_cast<decltype(oldcap)>(
                    std::count_if(std::begin(c), std::end(c),
                        [] (decltype(c)::const_reference obj) {
                        return (obj > std::make_tuple(3.14, 42, 17)); })));
        // check the other variants of comparison operators
        do_test(0 == static_cast<decltype(oldcap)>(
                    std::count_if(std::begin(c), std::end(c),
                        [] (decltype(c)::const_reference obj) {
                        return (std::make_tuple(3.14, 42, 17) > obj); })));
    }
    {
        // test insert(pos, first, last), erase(pos) and erase(first, last)
        // by comparing to an array-of-structures in a std::vector
        typedef std::size_t size_type;
        c.clear();
        do_test(c.empty());
        std::tuple<double, int, int> val(3.14, 0, 63);
        std::vector<std::tuple<double, int, int> > temp;
        temp.reserve(64);
        for (int i = 0; i < 64; ++i) {
            std::get<1>(val) = i;
            std::get<2>(val) = 63 - i;
            temp.push_back(val);
        }
        auto it = c.insert(c.begin(), temp.cbegin(), temp.cend());
        do_test(c.begin() == it);
        do_test(64 == c.size());
        do_test(c.size() == temp.size());
        do_test(temp.size() == std::inner_product(
                    c.begin(), c.end(), temp.begin(), size_type(0),
                    [] (size_type a, size_type b) { return a + b; },
                    [] (const decltype(val)& a, const decltype(val)& b) {
                        return size_type(a == b); }));

        // erase(pos)
        auto jt = temp.erase(temp.begin() + 3);
        do_test(temp.begin() + 3 == jt);
        auto kt = c.erase(c.begin() + 3);
        do_test(c.begin() + 3 == kt);
        do_test(c.size() == temp.size());
        do_test(temp.size() == std::inner_product(
                    c.begin(), c.end(), temp.begin(), size_type(0),
                    [] (size_type a, size_type b) { return a + b; },
                    [] (const decltype(val)& a, const decltype(val)& b) {
                        return size_type(a == b); }));

        // erase(first, last)
        auto lt = temp.erase(temp.begin() + 5, temp.begin() + 10);
        do_test(temp.begin() + 5 == lt);
        auto mt = c.erase(c.begin() + 5, c.begin() + 10);
        do_test(c.begin() + 5 == mt);
        do_test(c.size() == temp.size());
        do_test(temp.size() == std::inner_product(
                    c.begin(), c.end(), temp.begin(), size_type(0),
                    [] (size_type a, size_type b) { return a + b; },
                    [] (const decltype(val)& a, const decltype(val)& b) {
                        return size_type(a == b); }));

        // test sort (and swap)
        do_test(std::is_sorted(c.begin(), c.end(),
                [] (decltype(c)::value_type a,
                    decltype(c)::value_type b)
                { return std::get<1>(a) < std::get<1>(b); }));
        for (unsigned i = 0; i < temp.size(); ++i) {
            auto tmp = temp[i];
            std::printf("%2d temp (%f, %2d, %2d) ", i,
                    std::get<0>(tmp), std::get<1>(tmp), std::get<2>(tmp));
            tmp = c[i];
            std::printf("c (%f, %2d, %2d)\n",
                    std::get<0>(tmp), std::get<1>(tmp), std::get<2>(tmp));
        }
        std::printf("\n");
        std::sort(c.begin(), c.end(),
                [] (decltype(c)::value_type a,
                    decltype(c)::value_type b)
                { return std::get<1>(a) > std::get<1>(b); });
        std::sort(temp.begin(), temp.end(),
                [] (decltype(temp.front()) a, decltype(temp.front()) b)
                { return std::get<1>(a) > std::get<1>(b); });
        do_test(std::is_sorted(c.begin(), c.end(),
                [] (decltype(c)::value_type a,
                    decltype(c)::value_type b)
                { return std::get<1>(a) > std::get<1>(b); }));
        do_test(std::is_sorted(temp.begin(), temp.end(),
                [] (decltype(temp.front()) a, decltype(temp.front()) b)
                { return std::get<1>(a) > std::get<1>(b); }));
        do_test(c.size() == temp.size());
        for (unsigned i = 0; i < temp.size(); ++i) {
            auto tmp = temp[i];
            std::printf("%2d temp (%f, %2d, %2d) ", i,
                    std::get<0>(tmp), std::get<1>(tmp), std::get<2>(tmp));
            tmp = c[i];
            std::printf("c (%f, %2d, %2d)\n",
                    std::get<0>(tmp), std::get<1>(tmp), std::get<2>(tmp));
        }
        std::printf("\n");
        do_test(temp.size() == std::inner_product(
                    c.begin(), c.end(), temp.begin(), size_type(0),
                    [] (size_type a, size_type b) { return a + b; },
                    [] (const decltype(val)& a, const decltype(val)& b) {
                        return size_type(a == b); }));

        // test the begin<fieldno> and end<fieldno> calls
        do_test(&(*c.begin<0>()) == &(c.begin()->get<0>()));
        do_test(&(*cc.begin<0>()) == &(cc.begin()->get<0>()));
        do_test(&(*c.cbegin<0>()) == &(c.cbegin()->get<0>()));
        do_test(&(*c.end<0>()) == &(c.end()->get<0>()));
        do_test(&(*cc.end<0>()) == &(cc.end()->get<0>()));
        do_test(&(*c.cend<0>()) == &(c.cend()->get<0>()));
        // test (some of) the rbegin<fieldno> and rend<fieldno> calls
        do_test(&(*c.rbegin<0>()) == &(c.rbegin()->get<0>()));
        do_test(&(*c.rend<0>()) == &(c.rend()->get<0>()));
        // test the begin<fieldtag> and end<fieldtag> calls
        do_test(&(*c.begin<double>()) == &(c.begin()->get<double>()));
        do_test(&(*cc.begin<double>()) == &(cc.begin()->get<double>()));
        do_test(&(*c.cbegin<double>()) == &(c.cbegin()->get<double>()));
        do_test(&(*c.end<double>()) == &(c.end()->get<double>()));
        do_test(&(*cc.end<double>()) == &(cc.end()->get<double>()));
        do_test(&(*c.cend<double>()) == &(c.cend()->get<double>()));
        // test (some of) the rbegin<fieldno> and rend<fieldno> calls
        do_test(&(*c.rbegin<double>()) == &(c.rbegin()->get<double>()));
        do_test(&(*c.rend<double>()) == &(c.rend()->get<double>()));

        // rudimentary tests of comparison of containers
        decltype(c) d;
        decltype(temp) temp2;
        do_test(c == c);
        do_test(temp == temp);
        do_test(!(c != c));
        do_test(!(temp != temp));
        do_test(c != d);
        do_test(temp != temp2);
        do_test(!(c < c));
        do_test(!(temp < temp));
        do_test(d < c);
        do_test(temp2 < temp);
        do_test(!(c > c));
        do_test(!(temp > temp));
        do_test(!(d > c));
        do_test(!(temp2 > temp));
        do_test(c <= c);
        do_test(temp <= temp);
        do_test(c >= c);
        do_test(temp >= temp);
    }
    {
        // test assign(count, val)
        c.assign(42, std::make_tuple(3.14, 0, -1));
        do_test(42u == c.size());
        do_test(c.size() == std::size_t(std::count(std::begin(c), std::end(c),
                        std::make_tuple(3.14, 0, -1))));
        // assign(first, last) is just a frontend for clear(); insert(front,
        // end); - therefore, no test here
    }
    {
        // test emplace, emplace_back, resize
        c.clear();
        c.emplace_back(2.79, 42, 17);
        do_test(1 == c.size());
        do_test(c.front() == std::make_tuple(2.79, 42, 17));
        auto it = c.emplace(c.begin(), 2.79, 17, 42);
        do_test(2 == c.size());
        do_test(c.begin() == it);
        do_test(c.front() == std::make_tuple(2.79, 17, 42));
        do_test(c.back() == std::make_tuple(2.79, 42, 17));
        c.resize(64, std::make_tuple(3.14, 78, 17));
        do_test(64 == c.size());
        do_test(c.back() == std::make_tuple(3.14, 78, 17));
        c.resize(0);
        do_test(c.empty());
        c.resize(32);
        do_test(32 == c.size());
        const std::tuple<double, int, int> defaultval;
        do_test(c.back() == defaultval);
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
    class HitSkin : public NAKEDPROXY {
        public:
            template <typename... ARGS>
            HitSkin(ARGS&&... args) : NAKEDPROXY(std::forward<ARGS>(args)...) { }

            /// assignment operator - forward to underlying proxy
            template <typename ARG>
            HitSkin<NAKEDPROXY>& operator=(const ARG& arg) noexcept(noexcept(
                    static_cast<NAKEDPROXY*>(nullptr)->operator=(arg)))
            { NAKEDPROXY::operator=(arg); return *this; }

            /// move assignment operator - forward to underlying proxy
            template <typename ARG>
            HitSkin<NAKEDPROXY>& operator=(ARG&& arg) noexcept(noexcept(
                    static_cast<NAKEDPROXY*>(nullptr)->operator=(
                        std::move(arg))))
            { NAKEDPROXY::operator=(std::move(arg)); return *this; }

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
#include <cmath>
static void realistic_test() __attribute__((noinline));
static void realistic_test()
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

static void realistic_test_aos() __attribute__((noinline));
static void realistic_test_aos()
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

/// main program of unit test
int main()
{
    test();
    realistic_test();
    realistic_test_aos();
    std::printf("\n");
    if (0 == ntestsfail) std::printf("All tests passed.\n");
    else std::printf("Number of failed tests: %u\n", ntestsfail);
    return ntestsfail;
}
