#pragma once

#include <type_traits>

#include <Eigen/Core>
#include <Eigen/Geometry>
#include <nlohmann/json.hpp>

#include "Json2Numpy.hpp"

namespace Eigen {

template<typename Scalar>
void to_json(nlohmann::json &j, const MatrixBase<Scalar> &matrix) {

  auto m = matrix.eval();

  typename MatrixBase<Scalar>::Index rows = m.rows(), cols = m.cols();
  auto data_begin = const_cast<typename MatrixBase<Scalar>::Scalar *>(m.data());
  std::vector<typename MatrixBase<Scalar>::Scalar> v(data_begin, data_begin + rows * cols);

  bool is_row_major = matrix.IsRowMajor;

  j = {
      {"dtype", type::dtype<typename MatrixBase<Scalar>::Scalar>()},
      {"shape", {rows, cols}},
      {"data", v},
      {"order", is_row_major ? "C" : "F"}
  };
}

template<typename Scalar, int Rows, int Cols>
void from_json(const nlohmann::json &j, Matrix<Scalar, Rows, Cols> &matrix) {
  using MatrixType = Matrix<Scalar, Rows, Cols>;
  typename MatrixType::Index rows = matrix.rows(), cols = matrix.cols();
  bool is_row_major = matrix.IsRowMajor;

  assert(j["shape"][0].get<std::size_t>() == rows);
  assert(j["shape"][1].get<std::size_t>() == cols);
  assert(j["order"].get<std::string>() == (is_row_major ? "C" : "F"));

  Map<Matrix<Scalar, -1, 1>> flattened(matrix.data(), matrix.size());

  for (auto i = 0; i < flattened.size(); i++) {
    flattened(i) = j["data"][i].template get<Scalar>();
  }
}

// TODO, support deserialization to dynamic matrix.

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

TEST_CASE_TEMPLATE("static matrix/vector serialize/deserialize", T, Eigen::Matrix2d, Eigen::Matrix4f, Eigen::Vector3d, Eigen::Vector2d, Eigen::Vector4i
) {
  T m = T::Random();
  nlohmann::json json = m;
  T m1 = json.get<T>();
  CHECK_EQ(m, m1);
}

TEST_CASE_TEMPLATE("dynamic matrix serialize", T, Eigen::MatrixXd, Eigen::MatrixXf
) {
  std::size_t rows = 4;
  std::size_t cols = 3;
  T m(rows, cols);
  m.setRandom();
  nlohmann::json json = m;
  T m1(rows, cols);
  std::cout << m;
  std::cout << json.dump();

  CHECK_EQ(json["shape"][0], rows);
  CHECK_EQ(json["shape"][1], cols);

  for (int row = 0; row < rows; row++) {
    for (int col = 0; col < cols; col++) {
      auto i = row + col*rows;
      CHECK_EQ(json["data"][i].get<typename T::Scalar>(), m(row, col));
    }
  }

}

TEST_CASE_TEMPLATE("dynamic vector serialize", T, Eigen::VectorXd, Eigen::VectorXf
) {
  std::size_t rows = 4;
  T m(rows);
  m.setRandom();
  nlohmann::json json = m;
}

TEST_CASE_TEMPLATE("quaternion serialize/deserialize", Scalar, double, float) {
  Eigen::Quaternion<Scalar> q;
  Eigen::Quaternion<Scalar> q1;
  nlohmann::json json;
  q = Eigen::Quaternion<Scalar>::UnitRandom();
  json = q;
  q1 = json.get<Eigen::Quaternion<Scalar>>();
  CHECK_EQ(q.coeffs(), q1.coeffs());
}

}

#endif
