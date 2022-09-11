//
// Created by slovak on 8/30/22.
//

#pragma once

#include <unordered_map>
#include <functional>
#include <thread>
#include <string>

//#include <nlohmann/json.hpp>
#include <azmq/socket.hpp>
#include <spdlog/spdlog.h>

using namespace boost;
//using namespace nlohmann;

namespace remote {

namespace pubsub {

class Publisher {
 public:
  explicit Publisher(const std::string &addr, asio::io_service &ios)
      : pub_socket(ios) {

    pub_socket.bind(addr);

    spdlog::debug("Created Publisher with endpoint: {}", pub_socket.endpoint());

  }

  void Pub(const std::vector<uint8_t> &req) {
    this->pub_socket.send(azmq::message(asio::buffer(req)));
  }

  void APub(const std::vector<uint8_t> &req) {
    this->pub_socket.async_send(
        azmq::message(asio::buffer(req)),
        [](const boost::system::error_code &error, size_t bytes_received){
          spdlog::trace("Published bytes: {}", bytes_received);
          }, ZMQ_DONTWAIT);
  }

  azmq::pub_socket pub_socket;

};

class Subscriber {
 public:
  explicit Subscriber(const std::string &addr, asio::io_service &ios)
      : sub_socket(ios) {

    sub_socket.connect(addr);
    sub_socket.set_option(azmq::socket::subscribe(""));

    spdlog::debug("Created Subscriber with endpoint: {}", sub_socket.endpoint());

    m_buf.reserve(256);
    AsyncReceive();
  }

  void AsyncReceive() {

    spdlog::trace("Schedule async receive");

    this->m_buf.resize(1024);

    this->sub_socket.async_receive(
        asio::buffer(this->m_buf),
        [this](auto ...vn) { this->OnReceive(vn...); });
  }

  void OnReceive(const boost::system::error_code &error, size_t bytes_received) {

    spdlog::trace("Received bytes: {}", bytes_received);

    if (bytes_received) {
      this->m_buf.resize(bytes_received);
      spdlog::trace("Received: {}", this->m_buf[0]);
    }
    AsyncReceive();
  }

  std::vector<std::uint8_t> m_buf;
  azmq::sub_socket sub_socket;

};

}

}