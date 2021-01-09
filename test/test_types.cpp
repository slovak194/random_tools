
#include <iostream>
#include <fstream>
#include <memory>
#include <unordered_map>
#include <vector>
#include <type_traits>

#include "Server.h"


int main(int argc, char **argv) {


  std::ifstream i("/home/slovak/remote-config/config/conf_test.yaml.json");

  auto j = nlohmann::json::parse(i);

  remote_config::fix_arrays<int, float>(j);
  remote_config::check_numerical_homogenous_arrays(j);

  std::cout << remote_config::get_types(j).dump(1) << std::endl;

  return 0;
}
