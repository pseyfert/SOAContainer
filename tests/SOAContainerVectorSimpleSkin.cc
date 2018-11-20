/** @file tests/SOAContainerVectorSimpleSkin.cc
 *
 * @brief test SOAContainer with std::vector and simple skins
 *
 * @author Manuel Schiller <Manuel.Schiller@cern.ch>
 * @date 2015-04-11
 *
 * For copyright and license information, see the end of the file.
 */

#include <cmath>
#include <tuple>
#include <vector>
#include <random>
#include <chrono>

#include "gtest/gtest.h"
#include "SOAContainer.h"

namespace HitNamespace {
    namespace Fields {
        typedef struct : public SOA::Typelist::wrap_type<float> {} xAtYEq0;
        typedef struct : public SOA::Typelist::wrap_type<float> {} zAtYEq0;
        typedef struct : public SOA::Typelist::wrap_type<float> {} dxdy;
        typedef struct : public SOA::Typelist::wrap_type<float> {} dzdy;
        typedef struct : public SOA::Typelist::wrap_type<float> {} x;
        typedef struct : public SOA::Typelist::wrap_type<float> {} z;
        typedef struct : public SOA::Typelist::wrap_type<float> {} y;
    }

    template <typename NAKEDPROXY>
    class HitSkin : public SOA::PrintableNullSkin<NAKEDPROXY> {
        public:
            using fields_typelist =
                    SOA::Typelist::typelist<Fields::xAtYEq0, Fields::xAtYEq0,
                                            Fields::dxdy, Fields::dzdy,
                                            Fields::x, Fields::y, Fields::z>;
            using SOA::PrintableNullSkin<NAKEDPROXY>::PrintableNullSkin;
            using SOA::PrintableNullSkin<NAKEDPROXY>::operator=;

            auto xAtYEq0() const noexcept
                    -> decltype(this->template get<Fields::xAtYEq0>())
            {
                return this->template get<Fields::xAtYEq0>();
            }
            auto zAtYEq0() const noexcept
                    -> decltype(this->template get<Fields::zAtYEq0>())
            {
                return this->template get<Fields::zAtYEq0>();
            }
            auto x() const noexcept
                    -> decltype(this->template get<Fields::x>())
            {
                return this->template get<Fields::x>();
            }
            auto y() const noexcept
                    -> decltype(this->template get<Fields::y>())
            {
                return this->template get<Fields::y>();
            }
            auto z() const noexcept
                    -> decltype(this->template get<Fields::z>())
            {
                return this->template get<Fields::z>();
            }
            auto dxdy() const noexcept
                    -> decltype(this->template get<Fields::dxdy>())
            {
                return this->template get<Fields::dxdy>();
            }
            auto dzdy() const noexcept
                    -> decltype(this->template get<Fields::dzdy>())
            {
                return this->template get<Fields::dzdy>();
            }

            void setX(float x) noexcept
            {
                this->template get<Fields::x>() = x;
            }
            void setY(float y) noexcept
            {
                this->template get<Fields::y>() = y;
            }
            void setZ(float z) noexcept
            {
                this->template get<Fields::z>() = z;
            }

            auto x(float y) const noexcept
                    -> decltype(this->xAtYEq0() + this->dxdy() * y)
            {
                return xAtYEq0() + dxdy() * y;
            }
            auto z(float y) const noexcept
                    -> decltype(this->zAtYEq0() + this->dzdy() * y)
            {
                return zAtYEq0() + dzdy() * y;
            }

            float y(float y0, float ySl) const noexcept
            {
                return (y0 + zAtYEq0() * ySl) / (1 - dzdy() * ySl);
            }

            float updateHit(float y0, float ySl) noexcept
            {
                setY(y(y0, ySl));
                setZ(z(y())), setX(x(y()));
                return y();
            }
    };

    using namespace Fields;
    typedef SOA::Container<std::vector, HitSkin, xAtYEq0, zAtYEq0, dxdy, dzdy,
                           x, z, y>
            Hits;
    typedef typename Hits::reference Hit;
} // namespace HitNamespace

static void updateHits(HitNamespace::Hits& hits, float y0, float ySl)
        __attribute__((noinline));
static void updateHits(HitNamespace::Hits& hits, float y0, float ySl)
{
    using namespace HitNamespace;
    for (auto hit : hits) hit.updateHit(y0, ySl);
}

static void updateHits_v(HitNamespace::Hits& hits, float y0, float ySl)
        __attribute__((noinline));
static void updateHits_v(HitNamespace::Hits& hits, float y0, float ySl)
{
    using namespace HitNamespace;
    // give compiler a chance to not run out of registers, so it can
    // autovectorise
    for (auto hit : hits) hit.setY(hit.y(y0, ySl));
    for (auto hit : hits) {
        const auto y = hit.y();
        hit.setZ(hit.z(y)), hit.setX(hit.x(y));
    }
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
    AOSHit(float xAtYEq0, float zAtYEq0, float dxdy, float dzdy, float x,
           float z, float y)
            : m_xAtYEq0(xAtYEq0), m_zAtYEq0(zAtYEq0), m_dxdy(dxdy),
              m_dzdy(dzdy), m_x(x), m_z(z), m_y(y)
    {}

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
    {
        return (y0 + zAtYEq0() * ySl) / (1 - dzdy() * ySl);
    }

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

static void updateHits(AOSHits& hits, float y0, float ySl)
        __attribute__((noinline));
static void updateHits(AOSHits& hits, float y0, float ySl)
{
    for (auto& hit : hits) hit.updateHit(y0, ySl);
}

static void updateHits_v(AOSHits& hits, float y0, float ySl)
        __attribute__((noinline));
static void updateHits_v(AOSHits& hits, float y0, float ySl)
{
    // give compiler a chance to not run out of registers, so it can
    // autovectorise
    for (auto& hit : hits) hit.setY(hit.y(y0, ySl));
    for (auto& hit : hits) {
        const auto y = hit.y();
        hit.setZ(hit.z(y)), hit.setX(hit.x(y));
    }
}

TEST(SOAContainerVector, SimpleSkin)
{
    using namespace HitNamespace;
    Hits hits;
    hits.reserve(1024);
    AOSHits ahits;
    ahits.reserve(1024);
    for (unsigned i = 0; i < 1024; ++i) {
        float x0, z0, dxdy, dzdy, x, z, y;
        std::tie(x0, z0, dxdy, dzdy, x, z, y) = std::make_tuple(
                0.5f * i, 8500.f, std::tan(5.f / 180.f * float(M_PI)),
                3.6e-3f, 0.5f * i, 8500.f, 0.f);
        hits.emplace_back(x0, z0, dxdy, dzdy, x, z, y);
        ahits.emplace_back(x0, z0, dxdy, dzdy, x, z, y);
    }

    // test shifting the hits one by one
    for (unsigned i = 0; i < 1024; ++i) updateHits(hits, 300.f, -0.01f);
    for (unsigned i = 0; i < 1024; ++i) updateHits(ahits, 300.f, -0.01f);

    for (unsigned i = 0; i < 1024; ++i) {
        EXPECT_LT(std::abs(hits[i].x() - ahits[i].x()),
                  1e-6f * std::max(std::abs(hits[i].x()),
                                   std::abs(ahits[i].x())));
        EXPECT_LT(std::abs(hits[i].z() - ahits[i].z()),
                  1e-6f * std::max(std::abs(hits[i].z()),
                                   std::abs(ahits[i].z())));
        EXPECT_LT(std::abs(hits[i].y() - ahits[i].y()),
                  1e-6f * std::max(std::abs(hits[i].y()),
                                   std::abs(ahits[i].y())));
    }

    // test shifting the hits, hopefully autovectorised
    // heat up cache
    updateHits_v(hits, 300.f, -0.01f);
    const auto t0 = std::chrono::high_resolution_clock::now();
    for (unsigned i = 0; i < 10; ++i)
        updateHits_v(hits, 300.f, -0.01f);
    const auto t1 = std::chrono::high_resolution_clock::now();
    // heat up cache
    updateHits_v(ahits, 300.f, -0.01f);
    const auto t2 = std::chrono::high_resolution_clock::now();
    for (unsigned i = 0; i < 10; ++i)
        updateHits_v(ahits, 300.f, -0.01f);
    const auto t3 = std::chrono::high_resolution_clock::now();
    // make sure the vectorised version is faster
    const std::chrono::duration<double> dt0 = t1 - t0, dt1 = t3 - t2;
#if !defined(SANITIZE)
    EXPECT_LT(dt0.count(), dt1.count());
#endif // !SANITIZE

    for (unsigned i = 0; i < 1024; ++i) {
        EXPECT_LT(std::abs(hits[i].x() - ahits[i].x()),
                  1e-6f * std::max(std::abs(hits[i].x()),
                                   std::abs(ahits[i].x())));
        EXPECT_LT(std::abs(hits[i].z() - ahits[i].z()),
                  1e-6f * std::max(std::abs(hits[i].z()),
                                   std::abs(ahits[i].z())));
        EXPECT_LT(std::abs(hits[i].y() - ahits[i].y()),
                  1e-6f * std::max(std::abs(hits[i].y()),
                                   std::abs(ahits[i].y())));
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
