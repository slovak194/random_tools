
#include <chrono>

#include <boost/asio.hpp>

#include "ConfigLocal.h"
#include "Rpc.h"

using namespace std::chrono_literals;


void on_timer(boost::asio::steady_timer &timer, random_tools::config::ConfigLocal &conf) {
  timer.expires_after(std::chrono::seconds(1));

  auto m_const = json_eigen::MapMatrixXT<double, 3, 3>(conf("/test/matrix/data"));
  auto m_mutable = json_eigen::MapMatrixXT<std::int64_t>(conf["/test/vector/data"]);

  std::cout << m_const << std::endl;

//  m_const(0,0) += 1.0;
  m_mutable(0,0) += 1.0;

  timer.async_wait([&timer, &conf](const boost::system::error_code& error){
    on_timer(timer, conf);
  });
}


int main(int argc, char **argv) {

  spdlog::set_level(spdlog::level::debug);

  boost::asio::io_service ios;

  random_tools::config::ConfigLocal conf(std::string(PROJECT_SOURCE_DIR) + "/config/conf.yaml");

  random_tools::rpc::Server rpc("tcp://*:5555", ios);

  rpc.AddMethod("config_get", [&conf](json j) { return conf.Get(j["key"].get<std::string>()); });
  rpc.AddMethod("config_set", [&conf](json j) {
    conf.Set(j["key"].get<std::string>(), j["value"]);
    return conf.Get(j["key"].get<std::string>());
  });

//  boost::asio::steady_timer timer(ios);
//
//  timer.async_wait([&timer, &conf](const boost::system::error_code& error){
//    on_timer(timer, conf);
//  });

  ios.run();

  return 0;
}
