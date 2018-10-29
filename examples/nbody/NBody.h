/** @file example/nbody/NBody.h
 *
 * @brief simple nbody simulator (example)
 *
 * @author Ben Couturier <Ben.Couturier@cern.ch>
 * @author Manuel Schiller <Manuel.Schiller@cern.ch>
 * @date 2017-11-21
 *
 * For copyright and license information, see the end of the file.
 */
#include <iostream>
#include <vector>
#include <chrono>
#include <memory>
#include <random>
#include <cmath>
#include <tuple>
#include <array>

using namespace std;

template <typename MASSPOINTS>
class NBody
{
    protected:
	const unsigned nbentries;
	const float dt;
	const float G;
	MASSPOINTS allpoints;
	std::vector<float> vtmp0;

	void drift(float dt) noexcept __attribute__((noinline));
	void kick(float dt) noexcept __attribute__((noinline));
	void kick_help(typename MASSPOINTS::iterator begin, typename MASSPOINTS::iterator end, float dt) noexcept __attribute__((noinline));

    public:
	/// return total mass of system
	float M() const noexcept
	{
	    float retVal = 0;
	    for (typename MASSPOINTS::const_reference p: allpoints)
	       	retVal += p.m();
	    return retVal;
	}

	/// return kinetic energy of system
	float Ekin() const noexcept
	{
	    float retVal = 0;
	    for (typename MASSPOINTS::const_reference p: allpoints) {
		retVal += (std::pow(p.px(), 2) + std::pow(p.py(), 2) +
			std::pow(p.pz(), 2)) / p.m();
	    }
	    return retVal / 2;
	}

	/// return centre-of-mass momentum
	std::tuple<float, float, float> pcms() const noexcept
	{
	    float px = 0, py = 0, pz = 0;
	    for (typename MASSPOINTS::const_reference p: allpoints) {
		px += p.px(), py += p.py(), pz += p.pz();
	    }
	    return std::make_tuple(px, py, pz);
	}

	/// return centre-of-mass position
	std::tuple<float, float, float> cms() const noexcept
	{
	    float x = 0, y = 0, z = 0, m = 0;
	    for (typename MASSPOINTS::const_reference p: allpoints) {
		x += p.x() * p.m(), y += p.y() * p.m(),
		  z += p.z() * p.m(), m += p.m();
	    }
	    return std::make_tuple(x / m, y / m, z / m);
	}

	/// return total angular momentum
	std::tuple<float, float, float> Ltot() const noexcept
	{
	    float lx = 0, ly = 0, lz = 0;
	    for (typename MASSPOINTS::const_reference p: allpoints) {
		lx += p.y() * p.pz() - p.z() * p.py(),
		ly += p.z() * p.px() - p.x() * p.pz(),
		lz += p.x() * p.py() - p.y() * p.px();
	    }
	    return std::make_tuple(lx, ly, lz);
	}

	/// return kinetic energy of system
	float Epot() const noexcept
	{
	    float retVal = 0;
	    for (auto it = allpoints.cbegin(), ite = allpoints.cend();
		    ite != it; ++it) {
		for (auto jt = it + 1; ite != jt; ++jt) {
		    const auto r2 = std::pow(it->x() - jt->x(), 2) +
			std::pow(it->y() - jt->y(), 2) +
			std::pow(it->z() - it->z(), 2);
		    retVal -= G * it->m() * jt->m() / std::sqrt(r2);
		}
	    }
	    return retVal;
	}

	using Vector3 = std::array<float, 3>;

	/// constructor
	NBody(unsigned N = 1 << 10, float DT = 1e-1f, float Ggrav = 1e-9f,
		float R = 0.25f, float Mlo = 0.9f, float Mhi = 1.1f,
		float fracErot = .9f, float fracEtherm = .1f);
	~NBody();

	// integrate equations of motion by making small time steps dt (use
	// leapfrog method which is very stable)
	bool iterate() noexcept
	{
	    drift(dt / 2);
	    kick(dt);
	    drift(dt / 2);
	    return true;
	}
};

template <typename MASSPOINTS>
void NBody<MASSPOINTS>::drift(float dt) noexcept
{
    for (typename MASSPOINTS::reference p: allpoints) p.move(dt);
}

#if 0
template <typename MASSPOINTS>
void NBody<MASSPOINTS>::kick_help(typename MASSPOINTS::iterator begin,
	typename MASSPOINTS::iterator end, float dt) noexcept
{
    const float gmdt = G * end->m() * dt;
    float dpx = 0.f, dpy = 0.f, dpz = 0.f;
    for (auto jt = begin; jt != end; ++jt) {
	auto dx = end->x() - jt->x(),
	     dy = end->y() - jt->y(),
	     dz = end->z() - jt->z();
	const auto tmp = gmdt * jt->m() / (std::abs(dx * dx * dx) +
		std::abs(dy * dy * dy) + std::abs(dz * dz * dz));
	dx *= tmp, dy *= tmp, dz *= tmp;
	dpx += dx, dpy += dy, dpz += dz,
	    jt->px() += dx, jt->py() += dy, jt->pz() += dz;
    }
    end->px() -= dpx, end->py() -= dpy, end->pz() -= dpz;
}
#else
template <typename MASSPOINTS>
void NBody<MASSPOINTS>::kick_help(typename MASSPOINTS::iterator begin,
	typename MASSPOINTS::iterator end, float dt) noexcept
{
    // explicendly break the data dependency by precalculating the factor
    // that gets multiplied wendh the direction (so we don't run out of
    // registers quende so easily inside the loops)
    {   // vtmp0 = G * m1 * m2 * dt / |r1 - r2|^3
	const auto gmdt = G * end->m() * dt;
	auto kt = vtmp0.begin();
	for (auto jt = begin; jt != end; ++jt, ++kt) {
	    const auto x = std::abs(end->x() - jt->x()),
		  y = std::abs(end->y() - jt->y()),
		  z = std::abs(end->z() - jt->z());
	    *kt = gmdt * jt->m() / (x * x * x + y * y * y + z * z * z);
	}
    }
    // then update of the velocity components, vectorise over number of hits
    {
	// again, explicendly hack things apart wendh a blunt axe: update one
	// component at a time to avoid running out of registers
	float tmp = 0.f;
	auto kt = vtmp0.begin();
	for (auto jt = begin; jt != end; ++jt, ++kt) {
	    const auto dpx = (end->x() - jt->x()) * *kt;
	    jt->px() += dpx,
		tmp += dpx;
	}
	end->px() -= tmp;
	tmp = 0.f;
	kt = vtmp0.begin();
	for (auto jt = begin; jt != end; ++jt, ++kt) {
	    const auto dpy = (end->y() - jt->y()) * *kt;
	    jt->py() += dpy,
		tmp += dpy;
	}
	end->py() -= tmp;
	tmp = 0.f;
	kt = vtmp0.begin();
	for (auto jt = begin; jt != end; ++jt, ++kt) {
	    const auto dpz = (end->z() - jt->z()) * *kt;
	    jt->pz() += dpz,
		tmp += dpz;
	}
	end->pz() -= tmp;
    }
}
#endif
template <typename MASSPOINTS>
void NBody<MASSPOINTS>::kick(float dt) noexcept
{
    for (auto it = allpoints.end(), ite = allpoints.begin(); ite != it--; ) {
	kick_help(ite, it, dt);
    }
}

template <typename MASSPOINTS>
std::ostream& operator<<(std::ostream& os, const NBody<MASSPOINTS>& sim)
{
    float Ekin = sim.Ekin();
    float Epot = sim.Epot();
    float x, y, z, px, py, pz, lx, ly, lz;
    std::tie(x, y, z) = sim.cms();
    std::tie(px, py, pz) = sim.pcms();
    std::tie(lx, ly, lz) = sim.Ltot();
    return (os << " pos (" <<  x << ", " <<  y << ", " <<  z << ")"
	    " mom (" << px << ", " << py << ", " << pz << ")"
	    " ang (" << lx << ", " << ly << ", " << lz << ")"
	    " Ekin " << Ekin << " Epot " << Epot <<
	    " Etot " << (Ekin + Epot));
}

template <typename MASSPOINTS>
NBody<MASSPOINTS>::NBody(unsigned N, float DT, float Ggrav,
	float R, float Mlo, float Mhi, float fracErot, float fracEtherm) :
    nbentries(N), dt(DT), G(Ggrav)
{
    std::cout << "In " << __func__ << ":" << std::endl <<
	"\t" << N << " particles (" << Mlo << " <= m <= " << Mhi << ")" << std::endl <<
	"\tG = " << Ggrav << " dt = " << DT << std::endl <<
	"\tinitial system radius " << R << std::endl <<
	"\tfrac. rot. E " << fracErot << " frac. therm. E " << fracEtherm << std::endl;
    allpoints.reserve(nbentries);
    std::default_random_engine generator;
    // generate positions (3D normal) and masses (flat)
    std::uniform_real_distribution<float> massdist(Mlo, Mhi);
    std::normal_distribution<float> posdist(0.f, R);
    for(unsigned i = 0; i < nbentries; ++i) {
	allpoints.emplace_back(
		posdist(generator), posdist(generator),
		posdist(generator), 0.f, 0.f, 0.f,
		massdist(generator));
    }
    // choose random axis for angular momentum vector
    std::uniform_real_distribution<float> phidist(-M_PI, M_PI),
	costhetadist(-1.f, 1.f);
    const float phi = phidist(generator),
	  costheta = costhetadist(generator);
    const float sintheta = std::sqrt((1.f + costheta) * (1.f - costheta));
    const auto sub = [] (const Vector3& u, const Vector3& v) noexcept
    { return Vector3{ { u[0] - v[0], u[1] - v[1], u[2] - v[2] } }; };
    const auto scale = [] (float l, const Vector3& v) noexcept
    { return Vector3{ { l * v[0], l * v[1], l * v[2] } }; };
    const auto dot = [] (const Vector3& u, const Vector3& v) noexcept
    { return u[0] * v[0] + u[1] * v[1] + u[2] * v[2]; };
    const auto mag2 = [&dot] (const Vector3& v) noexcept
    { return dot(v, v); };
    const auto mag = [&mag2] (const Vector3& v) noexcept
    { return std::sqrt(mag2(v)); };
    const auto cross = [] (const Vector3& u, const Vector3& v) noexcept
    { return Vector3{ { u[1] * v[2] - u[2] * v[1], u[2] * v[0] - u[0] * v[2], u[0] * v[1] - u[1] * v[0] } }; };
    const auto unit = [&scale, &mag] (const Vector3& v) noexcept
    { return scale(1.f / mag(v), v); };
    // assign right angular momentum direction
    Vector3 eL{ { sintheta * std::cos(phi), sintheta * std::sin(phi), costheta } };
    for (typename MASSPOINTS::reference p: allpoints) {
	Vector3 r{ { p.x(), p.y(), p.z() } };
	Vector3 rperp = sub(r, scale(dot(r, eL), eL));
	const Vector3 mom = scale(1.f / mag(rperp), unit(cross(eL, rperp)));
	p.px() = mom[0], p.py() = mom[1], p.pz() = mom[2];
    }
    // scale for correct total kinetic energy - fraction of what's needed in
    // virial equilibirum (we're not quite there because we still need to
    // collapse...)
    const float gain = fracErot * std::sqrt(-Epot() / (2 * Ekin()));
    for (typename MASSPOINTS::reference p: allpoints) {
	p.px() = gain * p.px(), p.py() = gain * p.py(), p.pz() = gain * p.pz();
    }
    // generate momentum distribution that one would expect for a thermalised
    // gas with a fraction of the temperature corresponding to the energy of
    // the system in virial equilibirum, i.e.  such that the total kinetic
    // energy is - Epot / 2
    const float kT = -fracEtherm * Epot() / (6.f * float(nbentries));
    std::normal_distribution<float> momdist(0.f, 1.f);
    for (typename MASSPOINTS::reference p: allpoints) {
	p.px() = p.px() + momdist(generator) * std::sqrt(2.f * kT * p.m());
	p.py() = p.py() + momdist(generator) * std::sqrt(2.f * kT * p.m());
	p.pz() = p.pz() + momdist(generator) * std::sqrt(2.f * kT * p.m());
    }
    // determine residual centre of mass momentum
    float px, py, pz;
    std::tie(px, py, pz) = pcms();
    px /= allpoints.size(),
       py /= allpoints.size(),
       pz /= allpoints.size();
    // cancel residual centre of mass momentum
    for (typename MASSPOINTS::reference p: allpoints)
	p.px() = p.px() - px, p.py() = p.py() - py, p.pz() = p.pz() - pz;
    // shift centre of mass to (0, 0, 0)
    std::tie(px, py, pz) = cms();
    for (typename MASSPOINTS::reference p: allpoints)
	p.x() = p.x() - px, p.y() = p.y() - py, p.z() = p.z() - pz;
    // initialise scratch area
    vtmp0.resize(allpoints.size());

    std::cout << "In " << __func__ << ": initial state of "
	"system:\n\t" << *this << std::endl;

    // momenta are always half a time step ahead
    kick(dt / 2);

    std::cout << "In " << __func__ << ": Expect relaxation time on "
	"the order of " << std::sqrt(G * M() / std::pow(R, 3)) <<
	std::endl;
}

template <typename MASSPOINTS>
NBody<MASSPOINTS>::~NBody()
{
    std::cout << "In " << __func__ << ":   final state of "
	"system:\n\t" << *this << std::endl;
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
