// Created by s1nh.org..
// https://zhuanlan.zhihu.com/p/38136322

#include "sensor_data_interface.h"

#include <string>
#include <thread>

SensorDataInterface::SensorDataInterface() {
  num_img_ = 0;
}

void SensorDataInterface::InitVideoCapture(const std::vector<std::string>& video_file_names) {
  std::cout << "Initializing video capture..." << std::endl;

  num_img_ = video_file_names.size();

  // Init video capture.
  for (const auto& file_name : video_file_names) {
    cv::VideoCapture capture(file_name);
    if (!capture.isOpened())
      std::cout << "Failed to open capture " << file_name << std::endl;
    video_capture_vector_.push_back(capture);
  }
  std::cout << "Done. " << num_img_ << " captures initialized." << std::endl;
}

double SensorDataInterface::get_video_capture_fps() {
  if (!video_capture_vector_.empty()) {
    return video_capture_vector_[0].get(cv::CAP_PROP_FPS);
  }
  return 30.0;
}

cv::Size SensorDataInterface::get_video_capture_size() {
    if (!video_capture_vector_.empty()) {
        return cv::Size((int)video_capture_vector_[0].get(cv::CAP_PROP_FRAME_WIDTH),
                        (int)video_capture_vector_[0].get(cv::CAP_PROP_FRAME_HEIGHT));
    }
    return cv::Size(1920, 1080);
}

bool SensorDataInterface::get_next_frames(
    std::vector<cv::UMat>& image_vector) {
  for (size_t i = 0; i < num_img_; ++i) {
    cv::Mat frame;
    video_capture_vector_[i].read(frame);
    if (frame.empty()) {
      return false;
    }
    frame.copyTo(image_vector[i]);
  }
  return true;
}
