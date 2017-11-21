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

#endif // MPOINT_H
