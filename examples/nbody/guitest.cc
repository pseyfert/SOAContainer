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
	    markers = make_unique<TPolyMarker3D>(this->nbentries);
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



