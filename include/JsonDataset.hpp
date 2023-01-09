#pragma once

#include <fstream>
#include <string>
#include <nlohmann/json.hpp>

namespace json_dataset {

void dump_msg_append(const nlohmann::json &json, const std::string &dump_file_path) {
  const auto msgpack = nlohmann::json::to_msgpack(json);
  std::ofstream(dump_file_path, std::ios::app | std::ios::binary).write(
      reinterpret_cast<const char *>(msgpack.data()), msgpack.size() * sizeof(uint8_t));
}

void dump_msg(const nlohmann::json &json, const std::string &dump_file_path) {
  const auto msgpack = nlohmann::json::to_msgpack(json);
  std::ofstream(dump_file_path, std::ios::binary).write(
      reinterpret_cast<const char *>(msgpack.data()), msgpack.size() * sizeof(uint8_t));
}

void dump_txt(const nlohmann::json &json, const std::string &dump_file_path) {
  std::ofstream file(dump_file_path);
  file << json.dump(1);
}


auto load_msg(const std::string &dump_file_path) {
  return nlohmann::json::from_msgpack(std::ifstream(dump_file_path, std::ios::binary));
}

}
