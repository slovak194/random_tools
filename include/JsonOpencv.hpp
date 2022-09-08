//
// Created by slovak on 9/6/22.
//

#pragma once

// TODO, move to remote-tools

#include <opencv2/core.hpp>
#include <nlohmann/json.hpp>

using namespace nlohmann;

//void to_json(json &j, const Mat &m);
//void from_json(const json &j, cv::Mat &m);
//
//template<typename T>
//void to_json(json &j, const Point_<T> &p);
//
//template<typename T>
//void from_json(const json &j, Point_<T> &p);
//
//template<typename T>
//void to_json(json &j, const Point3_<T> &p);
//
//template<typename T>
//void from_json(const json &j, Point3_<T> &p);


namespace cv {

inline void to_json(json &j, const Mat &m) {
  json array = json::array();

  int cols = m.cols;
  int rows = m.rows;
  int depth = m.depth();

  if (m.isContinuous()) {
    cols *= rows,
        rows = 1;
  }

  switch (depth) {
    case CV_32F: {
      for (int row = 0; row < rows; row++) {
        auto vp = m.ptr<float>(row);
        for (int col = 0; col < cols; col++, vp++)
          array.push_back(*vp);
      }
      break;
    }
    case CV_64F: {
      for (int row = 0; row < rows; row++) {
        auto vp = m.ptr<double>(row);
        for (int col = 0; col < cols; col++, vp++)
          array.push_back(*vp);
      }
      break;
    }
  }
  j = json{
      {"shape", {m.rows, m.cols}},
      {"depth", m.depth()},
      {"type", m.type()},
      {"data", array}
  };
}

inline void from_json(const json &j, cv::Mat &m) {

  m = cv::Mat(j.at("/shape/0"_json_pointer), j.at("/shape/1"_json_pointer), j.at("/type"_json_pointer));

  int k = 0;

  switch (m.depth()) {
    case CV_32F: {
      for (int row = 0; row < m.rows; row++) {
        auto vp = m.ptr<float>(row);
        for (int col = 0; col < m.cols; col++, vp++) {
          *vp = j["data"][k++].get<float>();
        }
      }
      break;
    }
    case CV_64F: {
      for (int row = 0; row < m.rows; row++) {
        auto vp = m.ptr<double>(row);
        for (int col = 0; col < m.cols; col++, vp++) {
          *vp = j["data"][k++].get<double>();
        }
      }
      break;
    }
  }

}

template<typename T>
inline void to_json(json &j, const Point_<T> &p) {
  j = json{
      {"x", p.x},
      {"y", p.y}
  };
}

template<typename T>
inline void from_json(const json &j, Point_<T> &p) {
  p.x = j.at("x").get<T>();
  p.y = j.at("y").get<T>();
}

template<typename T>
inline void to_json(json &j, const Point3_<T> &p) {
  j = json{
      {"x", p.x},
      {"y", p.y},
      {"z", p.z}
  };
}

template<typename T>
inline void from_json(const json &j, Point3_<T> &p) {
  p.x = j.at("x").get<T>();
  p.y = j.at("y").get<T>();
  p.z = j.at("z").get<T>();
}

}
