//#include <remote_config/Server.h>

#include <iostream>

//#include "nlohmann/json.hpp"

#include "remote_config/Json2Eigen.hpp"


using json = nlohmann::json;


int main(int argc, char **argv) {

  json json;

  json["array"] = {1U, 2.0, 3.0, 4.0, 5.0};

  json["array_float"] = {1.0, 2.0, 3.0, 4.0, 5.0};
  json["array_unsigned"] = {1U, 2U, 3U, 4U, 5U};
  json["array_integer"] = {-1, -2, -3, -4, -5};
  json["array_boolean"] = {true, false, true, false, true};

//  std::cout << Eigen::MapMatrixXT<json::number_float_t>(json["array_float"], 1, 5) << std::endl;
//  std::cout << Eigen::MapVectorXT<json::number_unsigned_t>(json["array_unsigned"]) << std::endl;
//  std::cout << Eigen::MapRowVectorXT<json::number_integer_t>(json["array_integer"]) << std::endl;
//  std::cout << Eigen::MapMatrixXT<json::boolean_t>(json["array_boolean"], 5, 1) << std::endl;

  std::cout << static_cast<int>(json["array"].type()) << std::endl;
  std::cout << static_cast<int>(json["array"][0].type()) << std::endl;
  std::cout << static_cast<int>(json["array"][1].type()) << std::endl;



  return 0;
}
