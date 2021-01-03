#include <iostream>

#include <azmq/socket.hpp>
#include <boost/asio.hpp>
#include <array>
#include <chrono>
#include <thread>
#include <vector>
#include <fstream>
#include <memory>
#include <nlohmann/json.hpp>

using namespace std::chrono_literals;

namespace asio = boost::asio;

class ZmqHandler : public asio::io_service {
  std::shared_ptr<nlohmann::json> m_storage;
  std::mutex m_storage_mtx;
 public:
  azmq::rep_socket m_responder;
  std::vector<std::uint8_t> m_buf;

  explicit ZmqHandler(const std::string &config_path)
      : m_responder(*this) {
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

    std::thread thread([this]() { this->run(); });

    thread.detach();

  }

  void ScheduleReceive() {

    this->m_buf.resize(256);

    this->m_responder.async_receive(
        asio::buffer(this->m_buf),
        [this](auto ...vn) { this->OnReceive(vn...); });
  }

  void OnReceive(const boost::system::error_code &error, size_t bytes_transferred) {

    this->m_buf.resize(bytes_transferred);

    nlohmann::json req = nlohmann::json::from_msgpack(this->m_buf);
    std::cout << req.dump() << std::endl;

    nlohmann::json repl;

    if (req["type"].get<std::string>() == "get") {
      repl = this->Get(req["key"].get<std::string>());
    } else if (req["type"].get<std::string>() == "set") {
      repl = this->Set(req["key"].get<std::string>(), req["value"]);
    }

    this->m_responder.async_send(
        azmq::message(asio::buffer(nlohmann::json::to_msgpack(repl))),
        [this](auto ...vn) {});

    ScheduleReceive();
  }

  nlohmann::json Get(const std::string &key) {
    nlohmann::json tmp;
    {
      std::scoped_lock<std::mutex> lock(this->m_storage_mtx);
      tmp = (*this->m_storage)[nlohmann::json::json_pointer(key)];
    }
    return tmp;
  }

  nlohmann::json Set(const std::string &key, const nlohmann::json &value) {
    nlohmann::json tmp;
    {
      std::scoped_lock<std::mutex> lock(this->m_storage_mtx);
      (*this->m_storage)[nlohmann::json::json_pointer(key)] = value;
      tmp = (*this->m_storage)[nlohmann::json::json_pointer(key)];
    }
    return tmp;
  }

};

int main(int argc, char **argv) {

  ZmqHandler zmq_handler("/home/slovak/remote-config/config/conf_test.yaml");

  for (int i = 0; i < 1000; i++) {
    std::this_thread::sleep_for(1s);

    std::cout << "use config: " << zmq_handler.Get("/test/vector/data/0").dump() << std::endl;

  }

  return 0;
}