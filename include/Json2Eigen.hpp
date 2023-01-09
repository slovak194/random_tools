#pragma once

#include <type_traits>

#include <Eigen/Core>
#include <Eigen/Geometry>
#include <nlohmann/json.hpp>

#include "Json2Numpy.hpp"

namespace Eigen {

template<typename Scalar, int Rows, int Cols>
void to_json(nlohmann::json &j, const Matrix<Scalar, Rows, Cols> &matrix) {

  using MatrixType = Matrix<Scalar, Rows, Cols>;

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

template<typename Scalar, int Rows, int Cols>
void from_json(const nlohmann::json &j, Matrix<Scalar, Rows, Cols> &matrix) {

  using MatrixType = Matrix<Scalar, Rows, Cols>;
  typename MatrixType::Index rows = Rows, cols = Cols;
  bool is_row_major = matrix.IsRowMajor;

  assert(j["shape"][0].get<std::size_t>() == Rows);
  assert(j["shape"][1].get<std::size_t>() == Cols);
  assert(j["order"].get<std::string>() == (is_row_major ? "C" : "F"));

  Map<Matrix<Scalar, Rows * Cols, 1>> flattened(matrix.data(), matrix.size());

  for (auto i = 0; i < flattened.size(); i++) {
    flattened(i) = j["data"][i].template get<Scalar>();
  }
}

template<typename T>
void to_json(nlohmann::json &j, const Quaternion<T> &q) {

  j = {
      {"x", q.x()},
      {"y", q.y()},
      {"z", q.z()},
      {"w", q.w()},
  };
}

template<typename Scalar>
void from_json(const nlohmann::json &j, Quaternion<Scalar> &q) {
  q.x() = j["x"].get<Scalar>();
  q.y() = j["y"].get<Scalar>();
  q.z() = j["z"].get<Scalar>();
  q.w() = j["w"].get<Scalar>();
}

}

#ifdef DO_TESTS

#include <doctest.h>

TEST_SUITE("eigen") {

TEST_CASE_TEMPLATE("matrix/vector", T, Eigen::Matrix2d, Eigen::Matrix4f, Eigen::Vector3d, Eigen::Vector2d, Eigen::Vector4i
) {
  T m = T::Random();
  nlohmann::json json = m;
  T m1 = json.get<T>();
  CHECK(m == m1);

}

TEST_CASE_TEMPLATE("quaternion", Scalar, double, float) {
  Eigen::Quaternion<Scalar> q;
  Eigen::Quaternion<Scalar> q1;
  nlohmann::json json;
  q = Eigen::Quaternion<Scalar>::UnitRandom();
  json = q;
  q1 = json.get<Eigen::Quaternion<Scalar>>();
  CHECK(q.coeffs() == q1.coeffs());
}

}

#endif
