/** @file example/nbody/MPointSOA.h
 *
 * @brief SOA mass point for nbody simulation example
 *
 * @author Ben Couturier <Ben.Couturier@cern.ch>
 * @author Manuel Schiller <Manuel.Schiller@cern.ch>
 * @date 2017-11-21
 *
 * For copyright and license information, see the end of the file.
 */
#ifndef MPOINTSOA_H
#define MPOINTSOA_H

#include <math.h>
#include <iostream>

using namespace std;

#include "SOAContainer.h"

namespace SOAMPoint
{
    SOAFIELD_TRIVIAL(x, x, float);
    SOAFIELD_TRIVIAL(y, y, float);
    SOAFIELD_TRIVIAL(z, z, float);
    SOAFIELD_TRIVIAL(px, px, float);
    SOAFIELD_TRIVIAL(py, py, float);
    SOAFIELD_TRIVIAL(pz, pz, float);
    SOAFIELD_TRIVIAL(m, m, float);

    SOASKIN(Skin, x, y, z, px, py, pz, m) {
	SOASKIN_INHERIT_DEFAULT_METHODS(Skin);

	float dist() const noexcept
	{
	    return std::sqrt(this->x() * this->x() + this->y() * this->y() +
		    this->z() * this->z());
	}

	// more complicated methods
	void move(float dt) noexcept {
	    const auto dtm = dt / this->m();
	    this->x() = this->x() + dtm * this->px(),
	    this->y() = this->y() + dtm * this->py(),
	    this->z() = this->z() + dtm * this->pz();
	}
    };
}


typedef SOA::Container<std::vector, SOAMPoint::Skin> SOAMPoints;

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

#endif // MPOINT_H
