//
// Created by slovak on 8/30/22.
//

#pragma once

#include <nlohmann/json.hpp>
#include "yaml-cpp/yaml.h"

namespace json_yaml {

inline nlohmann::ordered_json yaml_to_json(const YAML::Node &node) {
  nlohmann::ordered_json json;

  if (node.IsScalar()) {
    try {
      return nlohmann::ordered_json::parse(node.as<std::string>());
    } catch (const nlohmann::detail::parse_error &e) {}

    try {
      return nlohmann::ordered_json::parse("[\"" + node.as<std::string>() + "\"]")[0];
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

inline YAML::Node json_to_yaml(const nlohmann::ordered_json& node) {
  return YAML::Load(node.dump());
}

}


#ifdef DO_TESTS
#include <iostream>

#include <doctest.h>

TEST_SUITE("json_yaml") {

TEST_CASE("yaml to json") {

  YAML::Node action_1;
  action_1["name"] = "add";
  action_1["counts"] = 1000;

  YAML::Node action_2;
  action_2["name"] = "idle";
  action_2["counts"] = 10000;

  YAML::Node local_item;
  local_item["name"] = "adder";
  local_item["action_counts"].push_back(action_1);
  local_item["action_counts"].push_back(action_2);

  YAML::Node local;
  local.push_back(local_item);

  YAML::Node subtree_item;
  subtree_item["name"] = "system";
  subtree_item["local"] = local;

  YAML::Node root_yaml;
  root_yaml["action_counts"]["version"] = "0.3";
  root_yaml["action_counts"]["subtree"].push_back(subtree_item);

  std::cout << root_yaml << std::endl;

  auto root_json = json_yaml::yaml_to_json(root_yaml);

  std::cout << root_json.dump(1) << std::endl;

  auto new_root_yaml = json_yaml::json_to_yaml(root_json);

  root_yaml.SetStyle(YAML::EmitterStyle::Flow);
  new_root_yaml.SetStyle(YAML::EmitterStyle::Flow);
  std::cout << root_yaml << std::endl;
  std::cout << new_root_yaml << std::endl;

  std::stringstream ss1;
  std::stringstream ss2;

  ss1 << root_yaml;
  ss2 << new_root_yaml;

  CHECK_EQ(ss1.str(), ss2.str());
}


}

#endif
