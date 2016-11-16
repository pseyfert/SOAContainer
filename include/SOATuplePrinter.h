/** @file SOATuplePrinter.h
 *
 * @author Henry Schreiner <Henry.Schreiner@cern.ch>
 * @date 2016-09-15
 */

// Based on recomendation of cppreference

#ifndef SOATUPLEPRINTER_H
#define SOATUPLEPRINTER_H

#include <iostream>
#include <tuple>

// helper function to print a tuple of any size

namespace TupleHelper {
template<class Tuple, std::size_t N>
struct TuplePrinter {
    static void print(std::ostream& os, const Tuple& t) {
        TuplePrinter<Tuple, N-1>::print(os, t);
        os << ", " << std::get<N-1>(t);
    }
};
 
template<class Tuple>
struct TuplePrinter<Tuple, 1> {
    static void print(std::ostream& os, const Tuple& t) {
        os << std::get<0>(t);
    }
};
 
template<class... Args>
void print_tuple(std::ostream& os, const std::tuple<Args...>& t) {
    os << "{";
    TuplePrinter<decltype(t), sizeof...(Args)>::print(os, t);
    os << "}";
}

template <typename... T>
std::ostream& operator<< (std::ostream& os, const std::tuple<T...> &a_tuple) {
    print_tuple(os, a_tuple);
    return os;
};

}

#endif // SOATUPLEPRINTER_H
// vim: sw=4:tw=78:ft=cpp