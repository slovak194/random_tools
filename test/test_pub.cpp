#include <thread>
#include <chrono>
#include <boost/asio.hpp>
#include <nlohmann/json.hpp>

#include "PubSub.h"

using namespace std::chrono_literals;
using namespace nlohmann;


void SetSigHandler(boost::asio::signal_set &sigs, asio::io_service &ios) {
  sigs.async_wait([&ios](const boost::system::error_code &error, int signal_number) {
    if (!error){
      if (signal_number == SIGINT) {
        spdlog::debug("On Signal: {} exiting ...", signal_number);
        ios.stop();
      }
    } else {
      spdlog::error(error.message());
    }
  });
}

int main(int argc, char **argv) {

  spdlog::set_level(spdlog::level::debug);

  asio::io_service ios;
  boost::asio::signal_set m_signals(ios, SIGINT, SIGUSR1);

  SetSigHandler(m_signals, ios);

  remote::pubsub::Publisher pub("tcp://127.0.0.1:5555", ios);

  std::thread th([&ios](){
    ios.run();
  });

  while (!ios.stopped()) {

    json j = {
        {"a", 1},
        {"b", 100},
        {"c", "casdcsad"},
    };

    pub.APub(json::to_msgpack(j));

//    spdlog::debug("Publishing ...");

    std::this_thread::sleep_for(std::chrono::seconds(1));
  }

  th.join();

  return 0;
}

