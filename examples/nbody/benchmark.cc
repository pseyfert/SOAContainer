/** @file example/nbody/benchmark.cc
 *
 * @brief simple nbody simulator (benchmarking example)
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


int main(int /* argc */, char * /* argv */ [])
{
    std::cout << "Running AOS code:" << std::endl;
    benchmark<MPoints>();
    std::cout << std::endl << "Running SOA code:" << std::endl;
    benchmark<SOAMPoints>();
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
 */
