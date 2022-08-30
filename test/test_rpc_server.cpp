
#include "Rpc.h"

using namespace std::chrono_literals;
using namespace nlohmann;

struct MyClass {
  nlohmann::json some(json j) {
    spdlog::debug("some called with args {}", j.dump());
    return j;
  }

  nlohmann::json other(json j) {
    spdlog::debug("other called with args {}", j.dump());
    return j;
  }
};

int main(int argc, char **argv) {

  spdlog::set_level(spdlog::level::debug);

  asio::io_service ios;

  MyClass my_class;

  remote::rpc::Server rpc("tcp://*:5555", ios);

  rpc.AddMethod("some", [&my_class](json a) { return my_class.some(a); });
  rpc.AddMethod("other", [&my_class](json a) { return my_class.other(a); });

  ios.run();

  return 0;
}
