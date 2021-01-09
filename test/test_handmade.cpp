//#include <remote_config/Server.h>

#include <iostream>
#include <vector>

struct MyNested {
  float some_float;
  int some_int;
  struct {
    unsigned some_unsigned;
    bool some_bool;
  } some_struct;

  std::vector<float> some_vector_float;

};


int main(int argc, char **argv) {


  MyNested mn = {
      10.0,
      -1,
      {
        100U,
        false
      },
      {1.0f, 2.0f, 3.0f, 4.0f}
  };

  return 0;
}
