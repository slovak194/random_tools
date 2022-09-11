//
// Created by slovak on 8/30/22.
//

#pragma once

#include <unordered_map>
#include <functional>
#include <thread>
#include <string>

#include <nlohmann/json.hpp>
#include <azmq/socket.hpp>
#include <spdlog/spdlog.h>

using namespace boost;
using namespace nlohmann;

namespace remote {

namespace rpc {

class Client {
 public:
  explicit Client(const std::string &addr, asio::io_service &ios)
      : req_socket(ios) {

    req_socket.connect(addr);
    m_buf.reserve(256);

    spdlog::debug("Created rpc client with endpoint: {}", addr);

  }

  template <typename... Ts>
  json Call(const std::string &fun, Ts const&... args) {
    json req;
    req["fun"] = fun;
    req["args"] = json::array({args...});
    return CallReq(req);
  }

  json CallReq(const json &req) {

    spdlog::debug("Sending {}", req.dump());
    this->req_socket.send(azmq::message(asio::buffer(json::to_msgpack(req))));
    this->m_buf.resize(1024);
    auto bytes_received = this->req_socket.receive(asio::buffer(this->m_buf));

    if (bytes_received) {
      this->m_buf.resize(bytes_received);
      spdlog::debug("Received bytes: {}", bytes_received);
      return json::from_msgpack(this->m_buf);
    } else {
      return {};
    }
  }

  std::vector<std::uint8_t> m_buf;
  azmq::req_socket req_socket;

};

class Server {
 public:
  explicit Server(const std::string &addr, asio::io_service &ios)
      : rep_socket(ios) {

    rep_socket.bind(addr);
    m_buf.reserve(256);

    spdlog::debug("Created rpc server with endpoint: {}", addr);

    Receive();
  }

  void Receive() {

    spdlog::debug("Schedule async receive");

    this->m_buf.resize(1024);

    this->rep_socket.async_receive(
        asio::buffer(this->m_buf),
        [this](auto ...vn) { this->OnReceive(vn...); });
  }

  void OnReceive(const boost::system::error_code &error, size_t bytes_transferred) {

    spdlog::debug("On Receive callback");

    json repl;

    if (error) {
      repl["error"] = error.message();
    } else {
      this->m_buf.resize(bytes_transferred);

      try {
        json req = json::from_msgpack(this->m_buf);

        spdlog::debug("Received: {}", req.dump());
        auto callable = calls.at(req["fun"].get<std::string>());
        auto args = req["args"];

        spdlog::debug("Calling calable with args: {}", args.dump());
        repl = callable(args);

      } catch (const std::exception &e) {
        repl["error"] = std::string(e.what());
      }
    }

    spdlog::debug("sending reply {}", repl.dump());
    this->rep_socket.async_send(
        azmq::message(asio::buffer(json::to_msgpack(repl))),
        [this](auto ...vn) {});

    Receive();
  }

  void AddMethod(const std::string &key, std::function<json(json)> call) {
    calls.emplace(key, call);
  }

  std::vector<std::uint8_t> m_buf;
  azmq::rep_socket rep_socket;

  std::unordered_map<std::string, std::function<json(json)>> calls;

};

}

}