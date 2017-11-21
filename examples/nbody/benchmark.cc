#include <iostream>
#include <vector>
#include <chrono>
#include <memory>
#include <random>
#include <cmath>

#include "NBody.h"
#include "MPoint.h"
#include "MPointSOA.h"

using namespace std;

template <typename MPOINTS>
void benchmark()
{
    using MassPoints = MPOINTS;

    auto t1 = chrono::high_resolution_clock::now();
    NBody<MassPoints> sim(1 << 12);
    auto t2 = chrono::high_resolution_clock::now();
    cout << "Initialization done ";
    cout << chrono::duration_cast<chrono::nanoseconds>(t2 - t1).count() << " ns\n";
    for (unsigned i = 0; i < 100; ++i) {
	sim.iterate();
    }
    auto t3 = chrono::high_resolution_clock::now();
    cout << "Iteration done ";
    cout << chrono::duration_cast<chrono::nanoseconds>(t3 - t2).count() << " ns\n";
}


int main(int argc, char *argv[])
{
    std::cout << "Running AOS code:" << std::endl;
    benchmark<MPoints>();
    std::cout << std::endl << "Running SOA code:" << std::endl;
    benchmark<SOAMPoints>();
    return 0;
}



