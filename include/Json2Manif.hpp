#pragma once

#include <type_traits>
#include <string>

#include <Eigen/Core>
#include <manif/manif.h>
#include <nlohmann/json.hpp>

#include "Json2Eigen.hpp"

namespace manif {

template <typename T>
using BaseType = typename std::remove_cv<typename std::remove_reference<T>::type>::type;

template <typename T> constexpr const char * ElName = "";

template <typename Scalar> inline constexpr const char * ElName<SO2<Scalar>> = "SO2";
template <typename Scalar> inline constexpr const char * ElName<SO3<Scalar>> = "SO3";
template <typename Scalar> inline constexpr const char * ElName<SE2<Scalar>> = "SE2";
template <typename Scalar> inline constexpr const char * ElName<SE3<Scalar>> = "SE3";

template <typename Scalar> inline constexpr const char * ElName<SO2Tangent<Scalar>> = "SO2Tangent";
template <typename Scalar> inline constexpr const char * ElName<SO3Tangent<Scalar>> = "SO3Tangent";
template <typename Scalar> inline constexpr const char * ElName<SE2Tangent<Scalar>> = "SE2Tangent";
template <typename Scalar> inline constexpr const char * ElName<SE3Tangent<Scalar>> = "SE3Tangent";

template <typename Scalar, unsigned int N> inline constexpr const char * ElName<Rn<Scalar, N>> = "Rn";
template <typename Scalar, unsigned int N> inline constexpr const char * ElName<RnTangent<Scalar, N>> = "RnTangent";

template<typename Scalar, template<typename> class ... Args>
inline constexpr const char * ElName<Bundle<Scalar, Args...>> = "Bundle";

template <typename T> inline constexpr const char * ElementName = ElName<BaseType<T>>;

template<typename G>
void to_json(nlohmann::json &j, const G &g) {
  j["type"] = ElementName<G>;
//  j["coeffs"] = g.coeffs();
  j["coeffs"] = std::vector(g.data(), g.data() + g.coeffs().size());
  assert(!j["type"].template get<std::string>().empty());
}

template<typename G>
void from_json(const nlohmann::json &j, G &g) {

  using Scalar = typename G::Scalar;
  constexpr int size = G::RepSize;
  for (auto i = 0; i < size; i++) {
    g.coeffs()(i) = j.at("coeffs")[i];
  }
}

template<typename G>
void from_json(const nlohmann::json &j, Eigen::Map<G> &g) {

  using Scalar = typename G::Scalar;
  constexpr int size = G::RepSize;
  for (auto i = 0; i < size; i++) {
    g.coeffs()(i) = j.at("coeffs")[i];
  }
}

template<typename G>
void to_json(nlohmann::json &j, const Eigen::Map<G> &g) {
    j = G(g);
}

template<typename Scalar, template<typename> class ... Args>
void to_json(nlohmann::json &j, const Bundle<Scalar, Args...> &bundle) {
  j["type"] = "Bundle";
  j["elements"] = nlohmann::json::array();
  j["coeffs"] = std::vector(bundle.data(), bundle.data() + bundle.coeffs().size());;

  using ThisBundle = Bundle<Scalar, Args...>;

  auto fill = [](nlohmann::json &j, std::size_t i, auto el) {
    j["elements"][i] = el;
  };

  auto lam = [&]<std::size_t... I>(std::index_sequence<I...>) {
    (fill(j, I, bundle.template element<I>()), ...);
  };

  lam(std::make_index_sequence<internal::traits<ThisBundle>::BundleSize>{});

  assert(!j["type"].template get<std::string>().empty());
}

}

