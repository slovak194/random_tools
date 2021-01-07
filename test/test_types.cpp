//#include <remote_config/Server.h>

#include <iostream>
#include <array>

#include "nlohmann/json.hpp"

void check_homogenous(const nlohmann::json &j) {

  if (j.is_array() && std::all_of(j.begin(), j.end(), [](const auto x) { return x.is_number() || x.is_boolean(); })) {
    if (!std::all_of(j.begin(), j.end(), [zero_type = j[0].type()](const auto x) { return x.type() == zero_type; })) {
      throw std::runtime_error("Elements in array with all numbers has different types");
    }
  } else if (!j.is_primitive()) {
    for (const auto &[k, v] : j.items()) {
      check_homogenous(v);
    }
  }
}

int main(int argc, char **argv) {

  nlohmann::json j;

  j["a"]["b"]["c"] = {1.0, 2.0, 3.0, 4.0, 5.0};
  j["array_unsigned"] = {1U, 2U, 3U, 4U, 5U};
  j["array_integer"] = {-1, -2, -3, -4, -5};
  j["array_boolean"] = {true, false, true, false, true};
  j["array_boolean"] = "fsdfa";

  std::cout << j.dump(1) << std::endl;

  check_homogenous(j);

  using my_json = nlohmann::basic_json<std::map, std::vector, std::string, bool, std::int64_t, std::int64_t, double>;

  my_json jsn;

  return 0;
}
