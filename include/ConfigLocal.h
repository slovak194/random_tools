//
// Created by slovak on 1/3/21.
//

#ifndef REMOTE_CONFIG_INCLUDE_SERVER_H_
#define REMOTE_CONFIG_INCLUDE_SERVER_H_

#include <iostream>
#include <array>
#include <vector>
#include <fstream>
#include <memory>

#include <Eigen/Core>
#include <Eigen/Dense>

#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

#include "JsonEigen.h"
#ifdef YAML_SUPPORT
#include "JsonYaml.h"
#endif

using namespace json_eigen;

namespace random_tools {
namespace config {

class ConfigLocal {
 public:

  std::string m_config_path;

 private:
  std::shared_ptr<nlohmann::json> m_storage;
  std::shared_ptr<nlohmann::json> m_types;

 public:
  explicit ConfigLocal(const std::string &config_path) {

    m_storage = std::make_shared<nlohmann::json>();
    m_types = std::make_shared<nlohmann::json>();

    Load(config_path);

  }

  nlohmann::json &operator[](const std::string &key) {
    return Get(key);
  }

  const nlohmann::json &operator()(const std::string &key) const {
    return GetConst(key);
  }

  template<typename T>
  T get(const std::string &key) {
    return GetConst(key).get<T>();
  }

  void Load(const std::string &config_path = "") {

    if (!config_path.empty()) {
      this->m_config_path = config_path;
    }

    nlohmann::json tmp;

    if (this->m_config_path.substr(this->m_config_path.find_last_of('.') + 1) == "json") {
      std::ifstream i(this->m_config_path);
      tmp = nlohmann::json::parse(i);
    }
#ifdef YAML_SUPPORT
    else if (this->m_config_path.substr(this->m_config_path.find_last_of('.') + 1) == "yaml") {
      tmp = json_yaml::yaml_to_json(YAML::LoadFile(this->m_config_path));
    } else if (this->m_config_path.substr(this->m_config_path.find_last_of('.') + 1) == "yml") {
      tmp = json_yaml::yaml_to_json(YAML::LoadFile(this->m_config_path));
    }
#endif
    else {
      throw std::runtime_error("Wrong file format");
    }

    fix_unsigned(tmp);
    fix_arrays<std::int64_t, double>(tmp);
    check_numerical_homogenous_arrays(tmp);

    this->Set(tmp);

    spdlog::debug(pprint(Get("")).dump(1));

  }

  nlohmann::json &Get(const std::string &key) {
    return (*this->m_storage).at(nlohmann::json::json_pointer(key));
  }

  const nlohmann::json &GetConst(const std::string &key) const {
    return (*this->m_storage).at(nlohmann::json::json_pointer(key));
  }

  const nlohmann::json &GetTypes(const std::string &key) {
    return (*this->m_types).at(nlohmann::json::json_pointer(key));
  }

  void Set(const nlohmann::json &value) {
    Set("", value, true);
  }

  void Set(const std::string &key, const nlohmann::json &value, bool unsafe = false) {
    auto new_types = get_types(value);
    auto old_types = GetTypes(key);

    if (unsafe) {
      (*this->m_storage).at(nlohmann::json::json_pointer(key)) = value;
      (*this->m_types).at(nlohmann::json::json_pointer(key)) = new_types;
    } else {

      auto diff = nlohmann::json::diff(old_types, new_types);
      if (diff.empty()) {
        (*this->m_storage).at(nlohmann::json::json_pointer(key)) = value;
      } else {

        auto updated_value = apply_types(value, old_types);
        new_types = get_types(updated_value);

        diff = nlohmann::json::diff(old_types, new_types);
        if (diff.empty()) {
          (*this->m_storage).at(nlohmann::json::json_pointer(key)) = updated_value;
        } else {
          throw std::runtime_error(
              std::string("Cannot cast new types to original types")
                  + "old: " + get_type_names(old_types).dump(1)
                  + ", new: " + get_type_names(new_types).dump(1)
          );
        }
      }
    }
  }

};

}
}

#endif //REMOTE_CONFIG_INCLUDE_SERVER_H_
