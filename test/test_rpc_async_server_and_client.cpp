#include <iostream>

#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_DEBUG
#define SPDLOG_DEBUG_ON
#define SPDLOG_DEBUG_ON
#define SPDLOG_FUNCTION __PRETTY_FUNCTION__

#include <spdlog/spdlog.h>

#include "Rpc.h"

using namespace std::chrono_literals;
using namespace nlohmann;

struct MyClass {
  nlohmann::json some(json j) {
    SPDLOG_DEBUG("[MAIN] some called with args {}", j.dump());
    return j;
  }

  nlohmann::json other(json j) {
    SPDLOG_DEBUG("[MAIN] other called with args {}", j.dump());
    return j;
  }
};

int main(int argc, char **argv) {

//  TODO, implement as DOCTEST suit

  spdlog::set_level(spdlog::level::debug);
  spdlog::set_pattern("%+ %!"); // Default pattern and function name, look here: https://github.com/gabime/spdlog/wiki/3.-Custom-formatting


  std::thread server_thread([]() {
    MyClass my_class;
    asio::io_service ios;
    random_tools::rpc::Server rpc("tcp://*:5555", ios);
    rpc.AddMethod("some", [&my_class](json a) { return my_class.some(a); });
    rpc.AddMethod("other", [&my_class](json a) { return my_class.other(a); });
    ios.run();
  });

  server_thread.detach();

  asio::io_service this_ios;

  std::thread client_thread([&this_ios]() {

    random_tools::rpc::Client rpc("tcp://127.0.0.1:5555", this_ios);

    for (auto i = 0; i < 1000; i++) {

      SPDLOG_DEBUG("[MAIN] Sending ...");

      json j;

      j[0] = 1;
      j[1] = 1.7;
      j[2] = "vsdfafvas";

      auto res = rpc.CallAsync("some", j);
      auto value = res.get();
      SPDLOG_DEBUG("[MAIN] Received result: {}", value.dump());

      assert(value[0] == j[0]);
      assert(value[1] == j[1]);
      assert(value[2] == j[2]);

      std::this_thread::sleep_for(1s);

    }

  });

  std::thread ios_thread([&this_ios]() {
    asio::io_context::work work(this_ios);
    this_ios.run();
    SPDLOG_DEBUG("[MAIN] this_ios exited.");
  });

  ios_thread.detach();
  
  client_thread.join();

  return 0;
}
