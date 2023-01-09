#pragma once

#include <utility>
#include <fstream>

#include <iostream>

#include <manif/manif.h>

#include <nlohmann/json.hpp>
#include <Json2Manif.hpp>

#include "enum.h"

namespace named_bundle {

template<typename S, typename T>
void print_element(T t, std::size_t i) {
  Eigen::IOFormat CommaInitFmt(Eigen::StreamPrecision, Eigen::DontAlignCols, ", ", ", ", "", "", "[", "]");
  std::cout << std::fixed << std::setprecision(3) << S::_from_index(i)._to_string() << ": " << t.coeffs().transpose().format(CommaInitFmt) << " ";
}

template<typename S, typename T, std::size_t... I>
void print(const T &state, std::index_sequence<I...>) {
  (print_element<S>(state.template element<I>(), I), ...);
}

template<typename S, typename T>
void print(const T &state) {
  print<S>(state, std::make_index_sequence<S::_size()>{});
  std::cout << "\n";
}

template<typename S, typename T>
auto to_json(const T &state) {
  nlohmann::json j;
  j["type"] = manif::ElementName<T>;
  j["elements"] = nlohmann::json::array();

  auto fill = [](nlohmann::json &j, std::size_t i, auto el) {
    j["elements"][i] = el;
    j["elements"][i]["name"] = S::_from_index(i)._to_string();
  };

  auto unroll = [&]<std::size_t... I>(std::index_sequence<I...>) {
    (fill(j, I, state.template element<I>()), ...);
  };

  unroll(std::make_index_sequence<manif::internal::traits<T>::BundleSize>{});

  assert(!j["type"].template get<std::string>().empty());

  return j;
}

template <typename T>
static constexpr std::size_t size = manif::internal::traits<T>::BundleSize;

}

#ifdef DO_TESTS

#include <doctest.h>

BETTER_ENUM(StateNames, std::size_t,
            Attitude,
            AngularVelocity,
            Position,
            LinearVelocity,
            Swing
)

TEST_SUITE("manif named") {


TEST_CASE("named bundle") {

  using State = manif::Bundle<
      double,
      manif::SO3,
      manif::R3,
      manif::R3,
      manif::R3,
      manif::SO2
  >;

  static_assert(StateNames::_size() == manif::internal::traits<State>::BundleSize);

  using at = StateNames;

  State state = State::Random();

  state.element<at::AngularVelocity>() = manif::R3d(Eigen::Vector3d(1, 2, 3));

  named_bundle::print<StateNames>(state);

  nlohmann::json json;
  json["state"] = named_bundle::to_json<StateNames>(state);

  std::cout << json.dump() << "\n";

  CHECK(json["state"]["elements"][1]["name"].get<std::string>() == "AngularVelocity");
  CHECK(json["state"]["elements"][1]["coeffs"][0].get<double>() == 1.0);
  CHECK(json["state"]["elements"][1]["coeffs"][1].get<double>() == 2.0);
  CHECK(json["state"]["elements"][1]["coeffs"][2].get<double>() == 3.0);

  std::string dump_file_path = "/home/slovak/kalman-benchmark/test/manifolds/to_json.msg";
  std::remove(dump_file_path.c_str());

  const std::vector<std::uint8_t> msgpack = nlohmann::json::to_msgpack(json);
  std::ofstream(dump_file_path, std::ios::app | std::ios::binary).write(
      reinterpret_cast<const char *>(msgpack.data()), msgpack.size() * sizeof(uint8_t));

  nlohmann::json json_new = nlohmann::json::from_msgpack(std::ifstream(dump_file_path, std::ios::binary));
  std::cout << json_new.dump() << "\n";
  CHECK(json_new == json);

  State state_new;

  state_new = json_new["state"].get<State>();

  CHECK(state == state_new);

}

}

#endif
