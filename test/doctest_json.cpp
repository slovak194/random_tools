#define DO_TESTS

#ifdef DO_TESTS
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include <Json2Eigen.hpp>
#include <Json2Manif.hpp>
#include <NamedBundle.hpp>

#else

int main () {
  return 0;
}

#endif
