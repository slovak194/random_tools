#include <iostream>

#include <azmq/socket.hpp>
#include <boost/asio.hpp>
#include <array>
#include <vector>
#include <nlohmann/json.hpp>

namespace asio = boost::asio;

class ZmqHandler {

 public:

  azmq::rep_socket m_responder;
  std::vector<std::uint8_t> m_buf;

  explicit ZmqHandler(asio::io_service &ios)
  : m_responder(ios) {
    m_responder.bind("tcp://127.0.0.1:5555");

    m_buf.reserve(256);

    ScheduleReceive();

  }

  void ScheduleReceive() {

    m_buf.resize(256);

    m_responder.async_receive(
        asio::buffer(m_buf),
        [this](auto ...vn) { this->OnReceive(vn...); });
  }

  void OnReceive(const boost::system::error_code &error, size_t bytes_transferred) {

//    std::cout << "some received bytes " << bytes_transferred << std::endl;

//    {"key":"/model","type":"get"}

    m_buf.resize(bytes_transferred);

    nlohmann::json req = nlohmann::json::from_msgpack(m_buf);
    std::cout << req.dump() << std::endl;

    nlohmann::json repl/* = "{'g': 10, 'm_c': 0.39, 'm_p': 0.614, 'h_p': 0.154, 'wheel_radius': 0.035, 'wheel_base': 0.183}"_json*/;

    repl["g"] = 10.0;
    repl["m_c"] = 0.39;

    const auto msgpack = nlohmann::json::to_msgpack(repl);

    azmq::message m(asio::buffer(msgpack));

    this->m_responder.async_send(m, [](auto ...vn){});

    ScheduleReceive();
  }

};



int main(int argc, char** argv) {

  asio::io_service ios;

  ZmqHandler zmq_handler(ios);

  ios.run();

  return 0;
}