//
// Created by slovak on 8/30/22.
//

#pragma once

#include <unordered_map>
#include <functional>
#include <thread>
#include <string>
#include <future>

#include <nlohmann/json.hpp>
#include <azmq/socket.hpp>
#include <spdlog/spdlog.h>

using namespace boost;
using namespace nlohmann;
using namespace std::chrono_literals;

namespace random_tools {

namespace rpc {

class Client {  // TODO, make async with black jack and futures.
 public:
  explicit Client(const std::string &addr, asio::io_service &ios, const std::chrono::milliseconds receive_timeout = 500ms)
      : m_req_socket(ios), m_receive_timeout(receive_timeout) {

    m_req_socket.connect(addr);
    m_buf.reserve(256);

    SPDLOG_DEBUG("[CLIENT] Created rpc client with endpoint: {}", addr);

  }

  void Cancel() {
    m_req_socket.cancel();
  }

  template <typename... Ts>
  json Call(const std::string &fun, Ts const&... args) {
    json req;
    req["fun"] = fun;
    req["args"] = json::array({args...});
//    return CallReq(req); // TODO, keep that for a plan B
    auto result_future = CallReqAsync(req);
    auto res_status = result_future.wait_for(m_receive_timeout);
      if (res_status == std::future_status::ready) {
        auto value = result_future.get();
        SPDLOG_DEBUG("[CLIENT] Received result: {}", value.dump());
        return value;
      } else {
        SPDLOG_WARN("[CLIENT] Receive receive_timeout, canceling ...");
        m_req_socket.cancel();
      }
    return {};
  }

  template <typename... Ts>
  std::future<json> CallAsync(const std::string &fun, Ts const&... args) {
    json req;
    req["fun"] = fun;
    req["args"] = json::array({args...});
    return CallReqAsync(req);
  }

  std::future<json> CallAsync(const std::string &fun, const json &args) {
    json req;
    req["fun"] = fun;
    req["args"] = args;
    return CallReqAsync(req);
  }

  json CallReq(const json &req) { // TODO, possibly deprecated

    SPDLOG_DEBUG("[CLIENT] Sending {}", req.dump());
    auto message = azmq::message(asio::buffer(json::to_msgpack(req)));
    this->m_req_socket.send(message);
    this->m_buf.resize(1024);
    auto bytes_received = this->m_req_socket.receive(asio::buffer(this->m_buf));

    if (bytes_received) {
      this->m_buf.resize(bytes_received);
      SPDLOG_DEBUG("[CLIENT] Received bytes: {}", bytes_received);
      return json::from_msgpack(this->m_buf);
    } else {
      return {};
    }
  }

  std::future<json> CallReqAsync(const json &req) {

    if (this->m_req_socket.get_io_service().stopped()) {
      SPDLOG_ERROR("[CLIENT] IO service is not running");
      exit(EXIT_FAILURE);
    }

    SPDLOG_DEBUG("[CLIENT] Sending async{}", req.dump());

    auto promise = std::make_shared<std::promise<json>>();
    std::future<json> future = promise->get_future();

    this->m_req_socket.async_send(
        azmq::message(asio::buffer(json::to_msgpack(req))),
        [this, promise](auto ...vn) {
          SPDLOG_DEBUG("[CLIENT] Rpc async rec sent, scheduling receive ...");

          auto buf = std::make_shared<std::vector<std::uint8_t>>(1024);
          this->m_req_socket.async_receive(
              asio::buffer(*buf),
              [buf, promise](const boost::system::error_code& ec, std::size_t bytes_received){
                SPDLOG_DEBUG("[CLIENT] error_code {} bytes_received {}", ec.message(), bytes_received);
                if (bytes_received) {
                  buf->resize(bytes_received);
                  auto result = json::from_msgpack(*buf);
                  SPDLOG_DEBUG("[CLIENT] Received bytes: {}, {}", bytes_received, result.dump());
                  promise->set_value(result);
                }
              });
        });

    return future;
  }

  azmq::message m_msg;
  std::vector<std::uint8_t> m_buf;
  azmq::req_socket m_req_socket;
  const std::chrono::milliseconds m_receive_timeout;

};

class Server {
 public:
  explicit Server(const std::string &addr, asio::io_service &ios)
      : rep_socket(ios) {

    rep_socket.bind(addr);
    m_buf.reserve(256);

    SPDLOG_DEBUG(" [SERVER] " "Created rpc server with endpoint: {}", addr);

    AddMethod("list", [this](const json &j) -> json {
      std::vector<std::string> keys;
      keys.reserve(calls.size());

      for(auto kv : calls) {
        keys.push_back(kv.first);
      }
      return keys;
    });

    Receive();
  }

  void Receive() {

    SPDLOG_DEBUG("[SERVER] Schedule async receive");

    this->m_buf.resize(1024);

    this->rep_socket.async_receive(
        asio::buffer(this->m_buf),
        [this](auto ...vn) { this->OnReceive(vn...); });
  }

  void OnReceive(const boost::system::error_code &error, size_t bytes_transferred) {

    SPDLOG_DEBUG("[SERVER] On Receive callback");

    json repl;

    if (error) {
      repl["error"] = error.message();
    } else {
      this->m_buf.resize(bytes_transferred);

      try {
        json req = json::from_msgpack(this->m_buf);

        SPDLOG_DEBUG("[SERVER] Received: {}", req.dump());
        auto callable = calls.at(req["fun"].get<std::string>());
        auto args = req["args"];

        SPDLOG_DEBUG("[SERVER] Calling calable with args: {}", args.dump());
        repl = callable(args);

      } catch (const std::exception &e) {
        repl["error"] = std::string(e.what());
      }
    }

    SPDLOG_DEBUG("[SERVER] Sending reply {}", repl.dump());
    this->rep_socket.async_send(
        azmq::message(asio::buffer(json::to_msgpack(repl))),
        [this](auto ...vn) {
          SPDLOG_DEBUG("[SERVER] Reply sent");
        });

    Receive();
  }

  void AddMethod(const std::string &key, std::function<json(json)> call) {
    calls.emplace(key, call);
  }

  std::vector<std::uint8_t> m_buf;
  azmq::rep_socket rep_socket;

  std::unordered_map<std::string, std::function<json(const json)>> calls;

};

}

}


#ifdef DO_TESTS
#include <iostream>
#include <doctest.h>
TEST_SUITE("some") {
TEST_CASE("else") {

}
}
#endif
