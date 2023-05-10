#define DO_TESTS

#ifdef DO_TESTS
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#define DOCTEST_CONFIG_SUPER_FAST_ASSERTS
#include "doctest.h"

#include <iostream>
#include <JsonYaml.h>


#else

int main () {
  return 0;
}

#endif
