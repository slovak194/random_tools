
#include <chrono>

#include "../include/Rpc.h"

using namespace std::chrono_literals;
using namespace nlohmann;

struct MyClass {
  nlohmann::json get(json j) {
    spdlog::debug("get called");
    spdlog::debug(j.dump());
    return j;
  }

  nlohmann::json set(json j) {
    spdlog::debug("set called");
    spdlog::debug(j.dump());
    return j;
  }

  nlohmann::json some(json j) {
    spdlog::debug("some called");
    spdlog::debug(j.dump());
    return j;
  }

  nlohmann::json other(json j) {
    spdlog::debug("other called");
    spdlog::debug(j.dump());
    return j;
  }
};

int main(int argc, char **argv) {

  spdlog::set_level(spdlog::level::debug);

  asio::io_service ios;

  MyClass my_class;

  Server rpc("tcp://*:5555", ios);

  rpc.AddMethod("get", [&my_class](json a) { return my_class.get(a); });
  rpc.AddMethod("set", [&my_class](json a) { return my_class.set(a); });
  rpc.AddMethod("some", [&my_class](json a) { return my_class.some(a); });
  rpc.AddMethod("other", [&my_class](json a) { return my_class.other(a); });

  ios.run();

  return 0;
}
