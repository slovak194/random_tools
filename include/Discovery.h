#pragma once

#include <chrono>

#include <boost/asio.hpp>
#include <spdlog/spdlog.h>

#include <nlohmann/json.hpp>

namespace discovery {


enum class ComponentType {
  Robot,
  Topside
};

NLOHMANN_JSON_SERIALIZE_ENUM( ComponentType, {
  {ComponentType::Robot, "Robot"},
  {ComponentType::Topside, "Topside"},
})

struct Component {
  static Component Robot(std::string name) {
    Component c;
    c.name = name;
    c.type = ComponentType::Robot;
    return c;
  }
  static Component Topside(std::string name) {
    Component c;
    c.name = name;
    c.type = ComponentType::Topside;
    return c;
  }
  std::string name;
  ComponentType type;
  std::string ip;
  nlohmann::json data;
};

void to_json(nlohmann::json &j, const Component &c) {
  j["name"] = c.name;
  j["type"] = c.type;
  j["ip"] = c.ip;
  j["data"] = c.data;
}

void from_json(const nlohmann::json &j, Component &c) {
  j.at("name").get_to(c.name);
  j.at("type").get_to(c.type);
  j.at("ip").get_to(c.ip);
  j.at("data").get_to(c.data);
}


class Listener {
 public:
  explicit Listener(boost::asio::io_service &ios, int port = 8000)
      : listening_socket(ios, boost::asio::ip::udp::v4()) {
    listening_socket.set_option(boost::asio::ip::udp::socket::reuse_address(true));
    listening_socket.bind(boost::asio::ip::udp::endpoint(boost::asio::ip::udp::v4(), port));
    m_buf.reserve(256);
    Listen();
  }

  void Listen() {
    this->m_buf.resize(1024);
    this->listening_socket.async_receive_from(
        boost::asio::buffer(this->m_buf),
        neighbor_endpoint,
        [this](auto ...vn) { this->OnReceive(vn...); });
  }

  void OnReceive(const boost::system::error_code &error, size_t bytes_transferred) {
    if (error) {
      SPDLOG_ERROR("{}", error.message());
      Listen();
      return;
    }

    this->m_buf.resize(bytes_transferred);
    nlohmann::json payload;

    try {
      payload = nlohmann::json::from_msgpack(this->m_buf);
    } catch (const std::exception &e) {
      SPDLOG_ERROR("{}", e.what());
    }

    auto ip = neighbor_endpoint.address().to_string();
    SPDLOG_TRACE("Received udp packet from {} with payload: {}", ip, payload.dump());
    Component component = payload;
    component.ip = ip;

    if (neighbors.find(component.name) == neighbors.end()) {
        SPDLOG_INFO("Added new neighbor {}", nlohmann::json(component).dump());
        neighbors[component.name] = component;
    } else {
      // TODO, OLSLO, check period.
    }
    Listen();
  }

  const std::string self_name;
  std::vector<std::uint8_t> m_buf;
  boost::asio::ip::udp::socket listening_socket;
  std::map<std::string, Component> neighbors;
  boost::asio::ip::udp::endpoint neighbor_endpoint;

};

class Broadcaster {
 public:
  explicit Broadcaster(boost::asio::io_service &ios, nlohmann::json payload, int broadcast_period_s = 1, int port = 8000) :
      broadcast_socket(ios, boost::asio::ip::udp::v4()),
      broadcast_timer(ios),
      broadcast_endpoint(boost::asio::ip::address_v4::broadcast(), port),
      broadcast_period(broadcast_period_s),
      payload(payload) {

    broadcast_socket.set_option(boost::asio::ip::udp::socket::reuse_address(true));
    broadcast_socket.set_option(boost::asio::socket_base::broadcast(true));
    ScheduleBroadcast();
  }

  void ScheduleBroadcast() {
    SPDLOG_TRACE("ScheduleBroadcast");
    broadcast_timer.expires_from_now(boost::posix_time::seconds(1));
    broadcast_timer.async_wait([this](auto ...vn) { this->Broadcast(); });
  }

  void Broadcast() {
    SPDLOG_TRACE("Broadcast");
    boost::system::error_code errcode;
    auto bytes_transfered = this->broadcast_socket.send_to(boost::asio::buffer(nlohmann::json::to_msgpack(payload)), this->broadcast_endpoint, 0, errcode);

    if (errcode.value() == boost::system::errc::success) {
      this->ScheduleBroadcast();
    } else {
      SPDLOG_ERROR("Exiting. Discovery broadcast failed with error: {}", errcode.message());
      exit(EXIT_FAILURE);
    }

    if (bytes_transfered == 0) {
      SPDLOG_ERROR("Exiting. Discovery broadcast bytes_transfered == 0");
      exit(EXIT_FAILURE);
    }

  }

  nlohmann::json payload;
  std::chrono::seconds broadcast_period;
  boost::asio::ip::udp::endpoint broadcast_endpoint;
  boost::asio::ip::udp::socket broadcast_socket;
  boost::asio::deadline_timer broadcast_timer;

};

class Endpoint {

 private:
  Broadcaster broadcaster;
  Listener listener;

 public:
  std::map<std::string, Component> &neighbors = listener.neighbors;

  Endpoint(boost::asio::io_service &ios, Component component)
      : broadcaster(ios, nlohmann::json(component)), listener(ios) {}

};

}
