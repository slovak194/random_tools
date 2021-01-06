//#include <remote_config/Server.h>

#include <iostream>

//#include "nlohmann/json.hpp"

#include "remote_config/Json2Eigen.hpp"

int main(int argc, char **argv) {

  nlohmann::json j;
  
  j["array_float"] = {1.0, 2.0, 3.0, 4.0, 5.0};
  j["array_unsigned"] = {1U, 2U, 3U, 4U, 5U};
  j["array_integer"] = {-1, -2, -3, -4, -5};
  j["array_boolean"] = {true, false, true, false, true};

  std::cout << Eigen::MapMatrixXT<nlohmann::json::number_float_t>(j["array_float"], 1, 5) << std::endl;
  std::cout << Eigen::MapVectorXT<nlohmann::json::number_unsigned_t>(j["array_unsigned"]) << std::endl;
  std::cout << Eigen::MapRowVectorXT<nlohmann::json::number_integer_t>(j["array_integer"]) << std::endl;
  std::cout << Eigen::MapMatrixXT<nlohmann::json::boolean_t>(j["array_boolean"], 5, 1) << std::endl;

  return 0;
}
