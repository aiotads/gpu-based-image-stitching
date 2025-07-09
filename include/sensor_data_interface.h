// Created by s1nh.org.

#ifndef IMAGE_STITCHING_SENSOR_DATA_INTERFACE_H
#define IMAGE_STITCHING_SENSOR_DATA_INTERFACE_H

#include <mutex>
#include <queue>
#include <vector>
#include <atomic>

#include <opencv2/opencv.hpp>

class SensorDataInterface {
public:

  SensorDataInterface();

  void InitVideoCapture(const std::vector<std::string>& video_file_names);

  bool get_next_frames(std::vector<cv::UMat>& image_vector);

  double get_video_capture_fps();
  cv::Size get_video_capture_size();

  size_t num_img_;

private:
  std::vector<cv::VideoCapture> video_capture_vector_;
};

#endif //IMAGE_STITCHING_SENSOR_DATA_INTERFACE_H
