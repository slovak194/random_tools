#pragma once

#include <type_traits>

#include <Eigen/Core>
#include <nlohmann/json.hpp>

#include "Json2Numpy.hpp"

namespace Eigen {
template<typename Scalar, int Rows, int Cols>
void to_json(nlohmann::json &j, const Eigen::Matrix<Scalar, Rows, Cols> &matrix) {

  using MatrixType = Eigen::Matrix<Scalar, Rows, Cols>;

  auto m = matrix.eval();

  typename MatrixType::Index rows = m.rows(), cols = m.cols();
  auto data_begin = const_cast<typename MatrixType::Scalar *>(m.data());
  std::vector<typename MatrixType::Scalar> v(data_begin, data_begin + rows * cols);

  bool is_row_major = matrix.IsRowMajor;

  j = {
      {"dtype", type::dtype<typename MatrixType::Scalar>()},
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

