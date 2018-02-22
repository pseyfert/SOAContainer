#include <array>
#include <iostream>
#include <tuple>
#include <vector>

#include <boost/container/small_vector.hpp>

#include "PrintableNullSkin.h"
#include "SOAContainer.h"

/// from google/benchmark
#define BENCHMARK_ALWAYS_INLINE __attribute__((always_inline))
template <class Tp>
inline BENCHMARK_ALWAYS_INLINE void DoNotOptimize(Tp const& value) {
  // Clang doesn't like the 'X' constraint on `value` and certain GCC versions
  // don't like the 'g' constraint. Attempt to placate them both.
#if defined(__clang__)
  asm volatile("" : : "g"(value) : "memory");
#else
  asm volatile("" : : "i,r,m"(value) : "memory");
#endif
}
///

namespace SOA
{
  namespace PointFields
  {
    using namespace SOA::Typelist;
    // since we can have more than one member of the same type in our
    // SOA object, we have to do some typedef gymnastics so the compiler
    // can tell them apart
    struct x : wrap_type<float> {
    };
    struct y : wrap_type<float> {
    };
  }

  template <typename NAKEDPROXY>
  class SOAPointProxy : public SOA::PrintableNullSkin<NAKEDPROXY>
  {
  public:
    using PrintableNullSkin<NAKEDPROXY>::PrintableNullSkin;
    using PrintableNullSkin<NAKEDPROXY>::operator=;
    using fields_typelist                        = SOA::Typelist::typelist<PointFields::x, PointFields::y>;

    float x() const noexcept { return this->template get<PointFields::x>(); }
    float y() const noexcept { return this->template get<PointFields::y>(); }
    void setX( float x ) noexcept { this->template get<PointFields::x>() = x; }
    void setY( float y ) noexcept { this->template get<PointFields::y>() = y; }

    // again, something beyond plain setters/getters
    float r2() const noexcept { return x() * x() + y() * y(); }
  };

  template <typename T>
  using smallv = boost::container::small_vector<T, 20>;
  // define the SOA container type
  typedef SOA::Container<std::vector, SOAPointProxy> PointsVector;
  typedef SOA::Container<smallv, SOAPointProxy> PointsSmallVector;
}

template<typename T>
auto fill_list_of_points() -> T {
  // Points list_of_points;
  T list_of_points;
  for (size_t i = 0 ; i < 20 ; ++i) list_of_points.emplace_back( (float)i, (float)5 );
  return list_of_points;
}


int main()
{

  std::cout << "filling a small_vector" << std::endl;
  for (size_t i = 0 ; i < 1e8 ; ++i) {
    auto f = fill_list_of_points<SOA::PointsSmallVector>();
    DoNotOptimize(f);
  }
  std::cout << "done" << std::endl;
  std::cout << "filling a standard vector" << std::endl;
  for (size_t i = 0 ; i < 1e8 ; ++i) {
    auto f = fill_list_of_points<SOA::PointsVector>();
    DoNotOptimize(f);
  }
  std::cout << "done" << std::endl;

  return 0;
}
