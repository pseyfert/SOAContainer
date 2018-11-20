/** @file example/nbody/guitest.cc
 *
 * @brief simple nbody simulator (ROOT gui example)
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

#include "MPointSOA.h"
#include "MPoint.h"
#include "NBody.h"

#include "TApplication.h"
#include "TCanvas.h"
#include "TTree.h"
#include "TList.h"
#include "TTimer.h"
#include "TRandom.h"
#include "TView.h"
#include "TPolyMarker3D.h"

template <typename MASSPOINTS>
class Animator: public TTimer, NBody<MASSPOINTS>
{
    public:
	Animator(int t) : TTimer(t), NBody<MASSPOINTS>() {  
	    markers = std::make_unique<TPolyMarker3D>(this->nbentries);
	    markers->SetMarkerSize(1);
	    markers->SetMarkerColor(kWhite);
	    markers->SetMarkerStyle(1);
	    markers->Draw();
	    // gPad owns copy of markers - save in markers2
	    markers2 = (TPolyMarker3D*) gPad->GetListOfPrimitives()->At(0);
	    gPad->SetFillColor(kBlack);
	}

	Bool_t Notify() {
	    if ((iterationCount & 0x7f) == 0) {
		std::cout << "time " << (iterationCount * this->dt) << *this <<
		    std::endl;
	    }
	    const bool retval = this->iterate();
	    if (0 == (iterationCount & 7)) {
		// ROOT and X are so slow at painting that we only update every
		// eight simulation steps
		{
		    int i = 0;
		    for(auto source: this->allpoints) {
			markers2->SetPoint(i, source.x(), source.y(), source.z());
			++i;
		    }
		}
		gPad->Modified();
		gPad->Update();
	    }
	    ++iterationCount;
	    return retval;
	}

    protected:
	unsigned iterationCount = 0;
	std::unique_ptr<TPolyMarker3D> markers;
	TPolyMarker3D* markers2;
};

int main(int argc, char *argv[])
{
    if (2 != argc || (0 != strcasecmp("soa", argv[1]) &&
		0 != strcasecmp("aos", argv[1]))) {
	std::cerr << "usage: " << argv[0] << " aos|soa" << std::endl;
	return 1;
    }
    TApplication *theApp = new TApplication("App", &argc, argv);
    TCanvas *c1 = new TCanvas("c1","TCanvas",100,100,700,700);
    TView *view = TView::CreateView(1);
    view->SetRange(-1, -1, -1, 1, 1, 1);
    c1->Draw();
    if (0 == strcasecmp(argv[1], "aos")) {
	std::cout << "running AOS simulation" << std::endl;
	Animator<MPoints> *timer = new Animator<MPoints>(0);
	timer->TurnOn();
	theApp->Run();
    } else {
	std::cout << "running SOA simulation" << std::endl;
	Animator<SOAMPoints> *timer = new Animator<SOAMPoints>(0);
	timer->TurnOn();
	theApp->Run();
    }
    return 0;
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

