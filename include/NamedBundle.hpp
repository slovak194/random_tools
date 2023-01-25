#pragma once

#include <utility>
#include <fstream>

#include <iostream>
#include <iomanip>

#include <manif/manif.h>

#include <nlohmann/json.hpp>
#include <Json2Manif.hpp>

#include "enum.h"

namespace named_bundle {

template<typename T>
static constexpr std::size_t size = manif::internal::traits<T>::BundleSize;

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

template<typename F, typename B, std::size_t... I>
void apply(F f, const B &bundle, std::index_sequence<I...>) {
  (f(bundle.template element<I>(), I), ...); // TODO, make variadic
}

template<typename F, typename B>
void apply(F f, const B &bundle) { // TODO, make variadic
  apply(f, bundle, std::make_index_sequence<size<B>>{});
}

}

#ifdef DO_TESTS

#include <doctest.h>

template <class T>
constexpr
std::string_view
type_name()
{
  using namespace std;
#ifdef __clang__
  string_view p = __PRETTY_FUNCTION__;
    return string_view(p.data() + 34, p.size() - 34 - 1);
#elif defined(__GNUC__)
  string_view p = __PRETTY_FUNCTION__;
#  if __cplusplus < 201402
  return string_view(p.data() + 36, p.size() - 36 - 1);
#  else
  return string_view(p.data() + 49, p.find(';', 49) - 49);
#  endif
#elif defined(_MSC_VER)
  string_view p = __FUNCSIG__;
    return string_view(p.data() + 84, p.size() - 84 - 7);
#endif
}

BETTER_ENUM(StateNames, std::size_t,
            Attitude,
            AngularVelocity,
            Position,
            LinearVelocity,
            Swing
)


TEST_SUITE("manif named") {

TEST_CASE("named apply") {
  using B = manif::Bundle<double, manif::SO3, manif::R4>;
  B::Tangent bt = B::Tangent::Zero();
  auto some_function = [](auto x, std::size_t i) { std::cout << x.coeffs() << std::endl; };
  named_bundle::apply(some_function, bt);
}

TEST_CASE("named check compact") {

  CHECK_EQ(manif::IsCompact<manif::SO3d>, true);
  CHECK_EQ(manif::IsCompact<manif::SO2d>, true);
  CHECK_EQ(manif::IsCompact<manif::R2d>, false);

  using B = manif::Bundle<double, manif::SO3, manif::R4>;
  B::Tangent bt = B::Tangent::Zero();

  CHECK_EQ(manif::IsCompact<decltype(bt.element<0>())::LieGroup>, true);
  CHECK_EQ(manif::IsCompact<decltype(bt.element<1>())::LieGroup>, false);

  auto twit = []<typename T>(T x, std::size_t i) {
    std::cout
    << x.coeffs().transpose() << " | "
    << T::DoF << " | "
    << type_name<typename T::LieGroup>() << " | "
    << std::string(manif::ElementName<typename T::LieGroup>) << " | "
    << manif::IsCompact<typename T::LieGroup>
    << std::endl; };

  named_bundle::apply(twit, bt);

//  bt.element<0>().coeffs()(0) = M_PI;


}

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

  CHECK_EQ(json["state"]["elements"][1]["name"].get<std::string>(), "AngularVelocity");
  CHECK_EQ(json["state"]["elements"][1]["coeffs"][0].get<double>(), 1.0);
  CHECK_EQ(json["state"]["elements"][1]["coeffs"][1].get<double>(), 2.0);
  CHECK_EQ(json["state"]["elements"][1]["coeffs"][2].get<double>(), 3.0);

  std::string dump_file_path = "/home/slovak/kalman-benchmark/test/manifolds/to_json.msg";
  std::remove(dump_file_path.c_str());

  const std::vector<std::uint8_t> msgpack = nlohmann::json::to_msgpack(json);
  std::ofstream(dump_file_path, std::ios::app | std::ios::binary).write(
      reinterpret_cast<const char *>(msgpack.data()), msgpack.size() * sizeof(uint8_t));

  nlohmann::json json_new = nlohmann::json::from_msgpack(std::ifstream(dump_file_path, std::ios::binary));
  std::cout << json_new.dump() << "\n";
  CHECK_EQ(json_new, json);

  State state_new;

  state_new = json_new["state"].get<State>();

  CHECK_EQ(state, state_new);

}

}

#endif
