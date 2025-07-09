// Created by s1nh.org.

#ifndef IMAGE_STITCHING_APP_H
#define IMAGE_STITCHING_APP_H

#include "opencv2/opencv.hpp"

#include "sensor_data_interface.h"
#include "image_stitcher.h"

class App {
public:
  App(const std::vector<std::string>& video_file_names);
  ~App();

  void run_stitching();

private:
  SensorDataInterface sensor_data_interface_;
  ImageStitcher image_stitcher_;
  cv::UMat image_concat_umat_;
  int total_cols_;
  cv::VideoWriter video_writer_;

};

#endif //IMAGE_STITCHING_APP_H
