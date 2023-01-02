#pragma once

#include <type_traits>

#include <Eigen/Core>
#include <manif/manif.h>
#include <nlohmann/json.hpp>

#include "Json2Eigen.hpp"

namespace manif {

template<typename Manifold>
void to_json(nlohmann::json &j, const Manifold &manifold) {

  // TODO
  j = manifold.coeffs().eval();
}
}
