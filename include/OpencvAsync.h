//
// Created by slovak on 9/19/22.
//

#pragma once

#include <thread>
#include <chrono>

#include "boost/asio.hpp"
#include "spdlog/spdlog.h"

#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>

#include "OpencvUtils.h"

using namespace boost;

namespace cv_utils {

static inline unsigned long get_thread_id() {
  return std::hash<std::thread::id>{}(std::this_thread::get_id());
}

struct MatMetaData {
  unsigned long capture_thread_id = 0U;
  double capture_id = 0U;
};

template<typename Fn, typename Executor>
class OpencvAsync {
 public:

  OpencvAsync(std::string filename, int api, Executor &executor, Fn &&fn)
      : filename(filename), api(api), executor(executor), fn(std::move(fn)) {
    executor.dispatch([this](auto ...vn) { this->Capture(vn...); });
  }

  std::string filename;
  int api;
  cv::VideoCapture cap;
  Executor &executor;
  Fn fn;

  void AsyncCapture() {
    spdlog::trace("Schedule async capture");
    executor.post([this](auto ...vn) { this->Capture(vn...); });
  }

  void Capture() {

    if (!cap.isOpened()) {
      spdlog::debug("[OpencvAsync] Creating capture object with uri: {}", filename);
      cap.open(filename, api);
      if (cap.isOpened()) {
        spdlog::debug("[OpencvAsync] Created capture object with uri: {}", filename);
      } else {
        spdlog::error("[OpencvAsync] Cannot create capture object with uri: {}", filename);
        std::this_thread::sleep_for(std::chrono::seconds(1));
      }
      AsyncCapture();
      return;
    }

    cv::Mat tmp;

    auto current_thread_id = get_thread_id();
    auto current_capture_id = cap.get(cv::CAP_PROP_POS_FRAMES);

    spdlog::trace("[OpencvAsync] {} capture frame [{} {}]", current_thread_id, current_capture_id, current_thread_id);

    auto res = cap.read(tmp);
    if (res) {
      auto meta_data = ((MatMetaData *) tmp.data);
      meta_data->capture_thread_id = current_thread_id;
      meta_data->capture_id = current_capture_id;
      spdlog::trace(
          "[OpencvAsync] Captured size: [{}, {}], type: {}",
          tmp.size().width, tmp.size().height, type2str(tmp.type()));
      fn(std::move(tmp));
    }

    AsyncCapture();
  }
};

}

