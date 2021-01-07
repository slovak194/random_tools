//
// Created by slovak on 1/3/21.
//

#ifndef REMOTE_CONFIG_INCLUDE_SERVER_H_
#define REMOTE_CONFIG_INCLUDE_SERVER_H_

#include <iostream>
#include <array>
#include <thread>
#include <vector>
#include <fstream>
#include <memory>

#include <azmq/socket.hpp>
#include <boost/asio.hpp>
#include <nlohmann/json.hpp>

#include "Json2Eigen.hpp"

namespace asio = boost::asio;

namespace remote_config {

nlohmann::json get_types(const nlohmann::json &j) {
  nlohmann::json types;
  auto jf = j.flatten();
  for (auto&[k, v] : jf.items()) {
    types[k] = static_cast<std::uint64_t>(v.type());
  }
  return types.unflatten();
}

void check_numerical_homogenous_arrays(const nlohmann::json &j) {

  if (j.is_array() && std::all_of(j.begin(), j.end(), [](const auto x) { return x.is_number() || x.is_boolean(); })) {
    if (std::any_of(j.begin(), j.end(), [zero_type = j[0].type()](const auto x) { return x.type() != zero_type; })) {
      throw std::runtime_error("Elements in all numbers array have different types: j = " + j.dump() + ", types: " + get_types(j).dump());
    }
  } else if (!j.is_primitive()) {
    for (const auto &[k, v] : j.items()) {
      check_numerical_homogenous_arrays(v);
    }
  }
}

class Server {

 public:

  std::string m_config_path;
  std::string m_tmp_json_file_path;

  explicit Server(asio::io_service &ios, const std::string &config_path, const std::string &addr = "tcp://127.0.0.1:5555")
      : m_responder(ios) {
    m_responder.bind(addr);

    m_buf.reserve(256);

    m_storage = std::make_shared<nlohmann::json>();
    m_types = std::make_shared<nlohmann::json>();

    Load(config_path);

    Receive();

  }

  template <typename T>
  auto Mat(const std::string &key, int rows, int cols) {
    return Eigen::MapMatrixXT<T>(Get(key), rows, cols);
  }

  template <typename T>
  auto CMat(const std::string &key, int rows, int cols) {
    return Eigen::MapMatrixXT<T>(GetConst(key), rows, cols);
  }

  const nlohmann::json& operator[](const std::string &key) {
    return Get(key);
  }

  nlohmann::json& operator()(const std::string &key) {
    return Get(key);
  }

 private:

  std::shared_ptr<nlohmann::json> m_storage;
  std::shared_ptr<nlohmann::json> m_types;

  azmq::rep_socket m_responder;
  std::vector<std::uint8_t> m_buf;

  void Load(const std::string &config_path = "") {

    if (!config_path.empty()) {
      this->m_config_path = config_path;
      this->m_tmp_json_file_path = config_path + ".json";
    }

    std::string command = std::string("python3 -c '") +
        std::string("config_file_yaml_path = \"") + this->m_config_path + std::string("\"\n") +
        "import yaml\n"
        "import json\n"
        "with open(config_file_yaml_path, \"r\") as yaml_in, open(\"" + this->m_tmp_json_file_path +
        "\", \"w\") as json_out:\n"
        "    yaml_object = yaml.safe_load(yaml_in)\n"
        "    json.dump(yaml_object, json_out)\n"
        "'";

    std::cout << std::endl << command << std::endl;

    auto res = system(command.c_str());

    if (res) {
      throw std::runtime_error("Conversion yaml to json has failed with exit code: " + std::to_string(res));
    }

    std::ifstream i(this->m_tmp_json_file_path);

//    std::ofstream o(this->m_tmp_json_file_path);
//    o << (*m_storage).dump(1, '\t');

    auto tmp = nlohmann::json::parse(i);
    check_numerical_homogenous_arrays(tmp);

    this->Set(tmp);

  }

  void Receive() {

    this->m_buf.resize(1024);

    this->m_responder.async_receive(
        asio::buffer(this->m_buf),
        [this](auto ...vn) { this->OnReceive(vn...); });
  }

  void OnReceive(const boost::system::error_code &error, size_t bytes_transferred) {

    this->m_buf.resize(bytes_transferred);

    nlohmann::json req = nlohmann::json::from_msgpack(this->m_buf);
    std::cout << req.dump() << std::endl;

    nlohmann::json repl;

    if (req["cmd"].get<std::string>() == "get") {
      repl = this->Get(req["key"].get<std::string>());
    } else if (req["cmd"].get<std::string>() == "set") {
      this->Set(req["key"].get<std::string>(), req["value"]);
      repl = this->Get(req["key"].get<std::string>());
    } else if (req["cmd"].get<std::string>() == "load") {
      this->Load(req["value"].get<std::string>());
      repl = this->Get("");
    }

    this->m_responder.async_send(
        azmq::message(asio::buffer(nlohmann::json::to_msgpack(repl))),
        [this](auto ...vn) {});

    Receive();
  }

  nlohmann::json& Get(const std::string &key) {
    return (*this->m_storage).at(nlohmann::json::json_pointer(key));
  }

  const nlohmann::json& GetConst(const std::string &key) {
    return (*this->m_storage).at(nlohmann::json::json_pointer(key));
  }

  const nlohmann::json& GetTypes(const std::string &key) {
    return (*this->m_types).at(nlohmann::json::json_pointer(key));  // TODO, OLSLO, catch!
  }

  void Set(const nlohmann::json &value) {
    Set("", value, true);
  }

  void Set(const std::string &key, const nlohmann::json &value, bool unsafe = false) {

    auto new_types = get_types(value);

    if (unsafe) {
      (*this->m_storage).at(nlohmann::json::json_pointer(key)) = value;  // TODO, OLSLO, catch!
      (*this->m_types).at(nlohmann::json::json_pointer(key)) = new_types;  // TODO, OLSLO, catch!
    } else {

      if (nlohmann::json::diff(GetTypes(key), new_types).empty()) {
        (*this->m_storage).at(nlohmann::json::json_pointer(key)) = value;  // TODO, OLSLO, catch!
      } else {
        std::cout << "Wrong type: " << std::endl; // TODO, OLSLO, introduce logging.
      }
    }
  }

};

}

#endif //REMOTE_CONFIG_INCLUDE_SERVER_H_
