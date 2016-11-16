#include <iostream>
#include <array>
#include <vector>
#include <tuple>

#include "SOAContainer.h"



int main() {

    SOAContainer<std::vector, NullSkin, double, int, int> c;
    c.emplace_back(1.2,4,5);
    const NullSkin<DressedTuple<std::tuple<const double &, const int &, const int &>, SOAContainer<std::vector, NullSkin, double, int, int> > > val1 = c.front();
    const NullSkin<DressedTuple<std::tuple<const double &, const int &, const int &>, SOAContainer<std::vector, NullSkin, double, int, int> > > val2 = c.back();
    
    val1 == val2;
    
    const SOAObjectProxy<SOAContainer<std::vector, NullSkin, double, int, int> >  val3 = c.front();
    const SOAObjectProxy<SOAContainer<std::vector, NullSkin, double, int, int> >  val4 = c.back();
    
    val4 == val3;
    
    
    return 0;
}