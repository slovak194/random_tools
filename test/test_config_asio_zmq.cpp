
#include <chrono>

#include <remote_config/Server.h>

using namespace std::chrono_literals;


void on_timer(boost::asio::steady_timer &timer, remote_config::Server &conf) {
  timer.expires_after(std::chrono::seconds(1));

  const auto m_const = conf.CMat<std::uint64_t>("/test/vector/data", 1, 3);
  auto m = conf.Mat<std::uint64_t>("/test/vector/data", 1, 3);

  std::cout << m_const << std::endl;

  m(0,0) += 1.0;

  timer.async_wait([&timer, &conf](const boost::system::error_code& error){
    on_timer(timer, conf);
  });
}


int main(int argc, char **argv) {

  boost::asio::io_service ios;

  remote_config::Server conf(ios, "/home/slovak/remote-config/config/conf_test.yaml");

  boost::asio::steady_timer timer(ios);

  timer.async_wait([&timer, &conf](const boost::system::error_code& error){
    on_timer(timer, conf);
  });

  ios.run();

  return 0;
}
