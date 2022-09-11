
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

  spdlog::set_level(spdlog::level::trace);

  asio::io_service ios;
  boost::asio::signal_set m_signals(ios, SIGINT, SIGUSR1);

  SetSigHandler(m_signals, ios);

  std::function<void(json)> cbk = [](const json j){ spdlog::debug(j.dump(1));};

  remote::pubsub::Subscriber sub("tcp://127.0.0.1:5002", cbk, ios);

  std::thread th([&ios](){
    ios.run();
  });

//  while (!ios.stopped()) {
//    spdlog::debug(sub.j.dump(1));
//    std::this_thread::sleep_for(std::chrono::milliseconds(50));
//  }

  th.join();

  return 0;
}
