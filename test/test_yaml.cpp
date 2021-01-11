#include <iostream>

#include "yaml-cpp/yaml.h"
#include "nlohmann/json.hpp"


nlohmann::json yaml_to_json(YAML::Node node) {
  nlohmann::json json;

  if (node.IsScalar()) {
    try {
      return nlohmann::json::parse(node.as<std::string>());
    } catch (const nlohmann::detail::parse_error &e) {}

    try {
      return nlohmann::json::parse("[\"" + node.as<std::string>() + "\"]")[0];
    } catch (const nlohmann::detail::parse_error &e) {}

    return node.as<std::string>();

  } else if (node.IsSequence()) {
    for (const auto& v : node) {
      json.push_back(yaml_to_json(v));
    }
  } else if (node.IsMap()) {
    for (const auto& v : node) {
      json[v.first.as<std::string>()] = yaml_to_json(v.second);
    }
  }
  return json;
}


int main(int argc, char **argv) {

  YAML::Node config = YAML::LoadFile("/home/slovak/remote-config/config/conf.yaml");

  nlohmann::json json = yaml_to_json(config);

  std::cout << json.dump(1) << std::endl;

  return 0;
}
