#include <iostream>

#include <azmq/socket.hpp>
#include <boost/asio.hpp>
#include <array>
#include <chrono>
#include <vector>
#include <fstream>
#include <memory>
#include <nlohmann/json.hpp>

namespace asio = boost::asio;

class ZmqHandler {

 public:

  std::shared_ptr<nlohmann::json> m_storage;
  azmq::rep_socket m_responder;
  std::vector<std::uint8_t> m_buf;

  explicit ZmqHandler(asio::io_service &ios, const std::string &config_path)
  : m_responder(ios) {
    m_responder.bind("tcp://127.0.0.1:5555");

    m_buf.reserve(256);

    const std::string tmp_json_file_path = config_path + ".json";

    std::string command = std::string("python3 -c '") +
        std::string("config_file_yaml_path = \"") + config_path + std::string("\"\n") +
        "import yaml\n"
        "import json\n"
        "with open(config_file_yaml_path, \"r\") as yaml_in, open(\"" + tmp_json_file_path +
        "\", \"w\") as json_out:\n"
        "    yaml_object = yaml.safe_load(yaml_in)\n"
        "    json.dump(yaml_object, json_out)\n"
        "'";

    std::cout << std::endl << command << std::endl;

    auto res = system(command.c_str());

    if (res) {
      throw std::runtime_error("Conversion yaml to json has failed with exit code: " + std::to_string(res));
    }

    std::ifstream i(tmp_json_file_path);
    m_storage = std::make_shared<nlohmann::json>();
    *m_storage = nlohmann::json::parse(i);

    std::ofstream o(tmp_json_file_path);

    o << (*m_storage).dump(1, '\t');

    ScheduleReceive();

  }

  void ScheduleReceive() {

    m_buf.resize(256);

    m_responder.async_receive(
        asio::buffer(m_buf),
        [this](auto ...vn) { this->OnReceive(vn...); });
  }

  void OnReceive(const boost::system::error_code &error, size_t bytes_transferred) {

    this->m_buf.resize(bytes_transferred);

    nlohmann::json req = nlohmann::json::from_msgpack(m_buf);
    std::cout << req.dump() << std::endl;

    nlohmann::json repl;

    if (req["type"].get<std::string>() == "get") {
      repl = (*this->m_storage)[nlohmann::json::json_pointer(req["key"].get<std::string>())];
    } else if (req["type"].get<std::string>() == "set") {
      (*this->m_storage)[nlohmann::json::json_pointer(req["key"].get<std::string>())] = req["value"];
      repl = (*this->m_storage)[nlohmann::json::json_pointer(req["key"].get<std::string>())];
    }

    const auto msgpack = nlohmann::json::to_msgpack(repl);

    azmq::message m(asio::buffer(msgpack));

    this->m_responder.async_send(m, [](auto ...vn){});

    ScheduleReceive();
  }

};



int main(int argc, char** argv) {

  asio::io_service ios;

  ZmqHandler zmq_handler(ios, "/home/slovak/remote-config/config/conf_test.yaml");

  ios.run();

  return 0;
}