#pragma once

#include <type_traits>
#include <string>

#include <Eigen/Core>
#include <manif/manif.h>
#include <nlohmann/json.hpp>

#include "Json2Eigen.hpp"

namespace manif {

template<typename T>
using BaseType = typename std::remove_cv<typename std::remove_reference<T>::type>::type;

template<typename T> constexpr const char *ElName = "";

template<typename Scalar> inline constexpr const char *ElName<SO2<Scalar>> = "manif_SO2";
template<typename Scalar> inline constexpr const char *ElName<SO3<Scalar>> = "manif_SO3";
template<typename Scalar> inline constexpr const char *ElName<SE2<Scalar>> = "manif_SE2";
template<typename Scalar> inline constexpr const char *ElName<SE3<Scalar>> = "manif_SE3";

template<typename Scalar> inline constexpr const char *ElName<SO2Tangent<Scalar>> = "manif_SO2Tangent";
template<typename Scalar> inline constexpr const char *ElName<SO3Tangent<Scalar>> = "manif_SO3Tangent";
template<typename Scalar> inline constexpr const char *ElName<SE2Tangent<Scalar>> = "manif_SE2Tangent";
template<typename Scalar> inline constexpr const char *ElName<SE3Tangent<Scalar>> = "manif_SE3Tangent";

template<typename Scalar, unsigned int N> inline constexpr const char *ElName<Rn<Scalar, N>> = "manif_Rn";
template<typename Scalar, unsigned int N> inline constexpr const char *ElName<RnTangent<Scalar, N>> = "manif_RnTangent";

template<typename Scalar, template<typename> class ... Args>
inline constexpr const char *ElName<Bundle<Scalar, Args...>> = "manif_Bundle";

template<typename Scalar, template<typename> class ... Args>
inline constexpr const char *ElName<BundleTangent<Scalar, Args...>> = "manif_BundleTangent";

template<typename T> inline constexpr const char *ElementName = ElName<BaseType<T>>;

template<typename ThisGroup>
void to_json(nlohmann::json &j, const ThisGroup &g) {
  j["type"] = ElementName<ThisGroup>;
  j["coeffs"] = std::vector(g.data(), g.data() + g.coeffs().size());
  assert(!j["type"].template get<std::string>().empty());
}

template<typename ThisGroup>
void from_json(const nlohmann::json &j, ThisGroup &g) {

  using Scalar = typename ThisGroup::Scalar;
  constexpr int size = ThisGroup::RepSize;
  for (auto i = 0; i < size; i++) {
    g.coeffs()(i) = j.at("coeffs")[i];
  }
}

template<typename ThisGroup>
void from_json(const nlohmann::json &j, Eigen::Map<ThisGroup> &g) {

  using Scalar = typename ThisGroup::Scalar;
  constexpr int size = ThisGroup::RepSize;
  for (auto i = 0; i < size; i++) {
    g.coeffs()(i) = j.at("coeffs")[i];
  }
}

template<typename ThisGroup>
void to_json(nlohmann::json &j, const Eigen::Map<ThisGroup> &g) {
  j = ThisGroup(g);
}

template<typename Scalar, template<typename> class ... Args>
void to_json(nlohmann::json &j, const Bundle<Scalar, Args...> &bundle) {
  using ThisBundle = Bundle<Scalar, Args...>;
  j["type"] = ElementName<ThisBundle>;
  j["elements"] = nlohmann::json::array();

  auto fill = [](nlohmann::json &j, std::size_t i, auto el) {
    j["elements"][i] = el;
  };
  auto unroll = [&]<std::size_t... I>(std::index_sequence<I...>) {
    (fill(j, I, bundle.template element<I>()), ...);
  };
  unroll(std::make_index_sequence<internal::traits<ThisBundle>::BundleSize>{});

  assert(!j["type"].template get<std::string>().empty());

}

template<typename Scalar, template<typename> class ... Args>
void to_json(nlohmann::json &j, const BundleTangent<Scalar, Args...> &bundle) {
  using ThisBundleTangent = BundleTangent<Scalar, Args...>;

  j["type"] = ElementName<ThisBundleTangent>;
  j["elements"] = nlohmann::json::array();

  auto fill = [](nlohmann::json &j, std::size_t i, auto el) {
    j["elements"][i] = el;
  };
  auto unroll = [&]<std::size_t... I>(std::index_sequence<I...>) {
    (fill(j, I, bundle.template element<I>()), ...);
  };
  unroll(std::make_index_sequence<internal::traits<ThisBundleTangent>::BundleSize>{});
}

template<typename Scalar, template<typename> class ... Args>
void from_json(const nlohmann::json &j, Bundle<Scalar, Args...> &bundle) {
  using ThisBundle = Bundle<Scalar, Args...>;
  auto unroll = [&]<std::size_t... I>(std::index_sequence<I...>) {
    ((bundle.template element<I>() = j["elements"][I].get<typename ThisBundle::template Element<I>>()), ...);
  };

  unroll(std::make_index_sequence<manif::internal::traits<ThisBundle>::BundleSize>{});
}

template<typename Scalar, template<typename> class ... Args>
void from_json(const nlohmann::json &j, BundleTangent<Scalar, Args...> &bundle) {
  using ThisBundleTangent = BundleTangent<Scalar, Args...>;
  auto unroll = [&]<std::size_t... I>(std::index_sequence<I...>) {
    ((bundle.template element<I>() = j["elements"][I].get<typename ThisBundleTangent::template Element<I>>()), ...);
  };

  unroll(std::make_index_sequence<manif::internal::traits<ThisBundleTangent>::BundleSize>{});
}

//template<typename S, typename T>
//void from_json(const nlohmann::json &j, T &state) {
//
//  auto unroll = [&]<std::size_t... I>(std::index_sequence<I...>) {
//    ((state.template element<I>() = j["elements"].get<T::template Element<I>>()), ...);
//  };
//
//  unroll(std::make_index_sequence<manif::internal::traits<T>::BundleSize>{});
//
//}


}

#ifdef DO_TESTS

#include <doctest.h>

TEST_SUITE("manif") {

TEST_CASE("traits") {
  std::cout << manif::ElementName<manif::Bundle<double, manif::SO3, manif::R3>> << std::endl;
  std::cout << manif::ElementName<manif::BundleTangent<double, manif::SO3, manif::R3>> << std::endl;

  std::string bundle = manif::ElementName<manif::Bundle<double, manif::SO3, manif::R3>>;
  std::string bundle_tangent = manif::ElementName<manif::BundleTangent<double, manif::SO3, manif::R3>>;

  std::cout << bundle << std::endl;
  std::cout << bundle_tangent << std::endl;

  CHECK_EQ(bundle, "manif_Bundle");
  CHECK_EQ(bundle_tangent, "manif_BundleTangent");
}

TEST_CASE("just bundle") {
  using ThisGroup = manif::Bundle<double, manif::SO3, manif::R3>;
  using ThisTangent = typename ThisGroup::Tangent;
  ThisGroup g, g1;
  ThisTangent t, t1;
  nlohmann::json json;
  g = ThisGroup::Random();
  json = g;
  CHECK_EQ(json["type"].get<std::string>(), "manif_Bundle");
  std::cout << json.dump() << "\n";
  g1 = json.get<ThisGroup>();
  CHECK_EQ(g, g1);
  t = ThisTangent::Random();
  json = t;
  std::string tangent_type = json["type"].get<std::string>();
  CHECK_EQ(tangent_type, "manif_BundleTangent");
  std::cout << json.dump() << "\n";
  t1 = json.get<ThisTangent>();
  CHECK_EQ(t, t1);
}

TEST_CASE_TEMPLATE("group and tangent", ThisGroup,
                   manif::SO3d,
                   manif::SO3f,
                   manif::SO2d,
                   manif::SO2f,
                   manif::R3d,
                   manif::R2d,
                   manif::R3f,
                   manif::Bundle<double, manif::SO3, manif::SO2, manif::R3, manif::R1>,
                   manif::Bundle<float, manif::SO3, manif::SO2, manif::R3, manif::R1>,
                   manif::R2f
) {
  using ThisTangent = typename ThisGroup::Tangent;
  ThisGroup g, g1;
  ThisTangent t, t1;
  nlohmann::json json;
  g = ThisGroup::Random();
  json = g;
  std::cout << json.dump() << "\n";
  g1 = json.get<ThisGroup>();
  CHECK_EQ(g, g1);
  t = ThisTangent::Random();
  json = t;
  std::cout << json.dump() << "\n";
  t1 = json.get<ThisTangent>();
  CHECK_EQ(t, t1);
  CHECK_EQ(t, t1);
}

}

#endif // DO_TESTS