#pragma once

#include <type_traits>

#include <Eigen/Core>
#include <nlohmann/json.hpp>

#include "Json2Numpy.hpp"

namespace Eigen {
template<typename T>
void to_json(nlohmann::json &j, const T &matrix) {

  auto m = matrix.eval();

  typename T::Index rows = m.rows(), cols = m.cols();
  auto data_begin = const_cast<typename T::Scalar *>(m.data());
  std::vector<typename T::Scalar> v(data_begin, data_begin + rows * cols);

  bool is_row_major = matrix.IsRowMajor;

  j = {
      {"dtype", type::dtype<typename T::Scalar>()},
      {"shape", {rows, cols}},
      {"data", v},
      {"order", is_row_major ? "C" : "F"}
  };
}

template<typename T>
void to_json(nlohmann::json &j, const Eigen::Quaternion<T> &q) {

  j = {
      {"x", q.x()},
      {"y", q.y()},
      {"z", q.z()},
      {"w", q.w()},
  };
}

}

