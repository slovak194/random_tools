//
// Created by slovak on 8/30/22.
//

#pragma once

#include <nlohmann/json.hpp>
#include "yaml-cpp/yaml.h"

namespace json_yaml {

inline nlohmann::json yaml_to_json(const YAML::Node &node) {
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
    for (const auto &v: node) {
      json.push_back(yaml_to_json(v));
    }
  } else if (node.IsMap()) {
    for (const auto &v: node) {
      json[v.first.as<std::string>()] = yaml_to_json(v.second);
    }
  }
  return json;
}

}
