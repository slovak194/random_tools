//
// Created by slovak on 8/30/22.
//

#pragma once

#include <unordered_map>
#include <functional>
#include <memory>
#include <thread>
#include <string>

#include <nlohmann/json.hpp>
#include <azmq/socket.hpp>
#include <spdlog/spdlog.h>

using namespace boost;
using namespace nlohmann;

class HomeMadeRpc {
 public:
  explicit HomeMadeRpc(const std::string &addr, asio::io_service &ios)
      : m_responder(ios) {

    m_responder.bind(addr);
    m_buf.reserve(256);
    Receive();
  }

  void Receive() {

    spdlog::debug("Receive");

    this->m_buf.resize(1024);

    this->m_responder.async_receive(
        asio::buffer(this->m_buf),
        [this](auto ...vn) { this->OnReceive(vn...); });
  }

  void OnReceive(const boost::system::error_code &error, size_t bytes_transferred) {

    spdlog::debug("On Receive");

    json repl;

    if (error) {
      repl["error"] = error.message();
    } else {
      this->m_buf.resize(bytes_transferred);

      try {
        json req = json::from_msgpack(this->m_buf);

        spdlog::debug(req.dump());

//        if (auto it = calls.find(req["cmd"].get<std::string>()); it != calls.end()) {
//          auto res = it->second(req["value"]);
//        }

        repl = calls.at(req["fun"].get<std::string>())(req["args"]);

//        if (req["cmd"].get<std::string>() == "get") {
//          repl = this->Get(req["key"].get<std::string>());
//        } else if (req["cmd"].get<std::string>() == "set") {
//          this->Set(req["key"].get<std::string>(), req["value"]);
//          repl = this->Get(req["key"].get<std::string>());
//        } else if (req["cmd"].get<std::string>() == "load") {
//          this->Load(req["value"].get<std::string>());
//          repl = this->Get("");
//        }
      } catch (const std::exception &e) {
        repl["error"] = std::string(e.what());
      }
    }

    this->m_responder.async_send(
        azmq::message(asio::buffer(json::to_msgpack(repl))),
        [this](auto ...vn) {});

    Receive();
  }

  void AddMethod(const std::string &key, std::function<json(json)> call) {
    calls.emplace(key, call);
  }

  std::vector<std::uint8_t> m_buf;
  azmq::rep_socket m_responder;

  std::unordered_map<std::string, std::function<json(json)>> calls;

};
