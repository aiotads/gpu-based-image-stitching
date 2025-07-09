// Created by s1nh.org.

#include "app.h"

#include <iostream>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include "image_stitcher.h"
#include "stitching_param_generater.h"

App::App(const std::vector<std::string>& video_file_names) {
  sensor_data_interface_.InitVideoCapture(video_file_names);

  std::vector<cv::UMat> first_image_vector = std::vector<cv::UMat>(sensor_data_interface_.num_img_);
  std::vector<cv::Mat> first_mat_vector = std::vector<cv::Mat>(sensor_data_interface_.num_img_);
  std::vector<cv::UMat> reproj_xmap_vector;
  std::vector<cv::UMat> reproj_ymap_vector;
  std::vector<cv::UMat> undist_xmap_vector;
  std::vector<cv::UMat> undist_ymap_vector;
  std::vector<cv::Rect> image_roi_vect;

  // Get the first frame to initialize stitching parameters
  if (!sensor_data_interface_.get_next_frames(first_image_vector)) {
      std::cerr << "Error: Could not read initial frames from video sources." << std::endl;
      exit(1);
  }

  for (size_t i = 0; i < sensor_data_interface_.num_img_; ++i) {
    first_image_vector[i].copyTo(first_mat_vector[i]);
  }

  StitchingParamGenerator stitching_param_generator(first_mat_vector);

  stitching_param_generator.GetReprojParams(
      undist_xmap_vector,
      undist_ymap_vector,
      reproj_xmap_vector,
      reproj_ymap_vector,
      image_roi_vect
  );

  image_stitcher_.SetParams(
      100,
      undist_xmap_vector,
      undist_ymap_vector,
      reproj_xmap_vector,
      reproj_ymap_vector,
      image_roi_vect
  );
  total_cols_ = 0;
  for (size_t i = 0; i < sensor_data_interface_.num_img_; ++i) {
    total_cols_ += image_roi_vect[i].width;
  }
  image_concat_umat_ = cv::UMat(image_roi_vect[0].height, total_cols_, CV_8UC3);

  double fps = sensor_data_interface_.get_video_capture_fps();
  cv::Size frame_size = sensor_data_interface_.get_video_capture_size();
  video_writer_.open("../results/stitched_video.mp4", cv::VideoWriter::fourcc('H', '2', '6', '4'), fps, cv::Size(total_cols_, image_roi_vect[0].height));
}

App::~App() {
  if (video_writer_.isOpened()) {
    video_writer_.release();
  }
}

void App::run_stitching() {
  std::vector<cv::UMat> image_vector(sensor_data_interface_.num_img_);
  std::vector<cv::UMat> images_warped_vector(sensor_data_interface_.num_img_);

  int64_t t0, t1, t2, tn;

  size_t frame_idx = 0;
  while (sensor_data_interface_.get_next_frames(image_vector)) {
    t0 = cv::getTickCount();

    std::vector<std::thread> warp_thread_vect;
    t1 = cv::getTickCount();

    for (size_t img_idx = 0; img_idx < sensor_data_interface_.num_img_; ++img_idx) {
      warp_thread_vect.emplace_back(
          &ImageStitcher::WarpImages,
          &image_stitcher_,
          img_idx,
          20,
          image_vector[img_idx],
          std::ref(images_warped_vector),
          std::ref(image_concat_umat_)
      );
    }
    for (auto& warp_thread : warp_thread_vect) {
      warp_thread.join();
    }
    t2 = cv::getTickCount();

    double fps = 1 / (double(t2 - t0) / cv::getTickFrequency());
    double real_fps = 1 / (double(cv::getTickCount() - t0) / cv::getTickFrequency());
    std::string fps_text = "FPS: " + std::to_string(fps);
    std::string real_fps_text = "Real FPS: " + std::to_string(real_fps);

    cv::Mat frame_to_write;
    image_concat_umat_.copyTo(frame_to_write);
    cv::putText(frame_to_write, fps_text, cv::Point(10, 50), cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(0, 255, 0), 2);
    cv::putText(frame_to_write, real_fps_text, cv::Point(10, 100), cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(0, 255, 0), 2);

    if (video_writer_.isOpened()) {
        video_writer_.write(frame_to_write);
    }

    frame_idx++;
    tn = cv::getTickCount();

    std::cout << "[app] "
              << double(t1 - t0) / cv::getTickFrequency() << ";"
              << double(t2 - t1) / cv::getTickFrequency() << ";"
              << fps << " FPS; "
              << real_fps << " Real FPS." << std::endl;
  }
}

int main(int argc, char* argv[]) {
    std::vector<std::string> video_files;
    for (int i = 1; i < argc; ++i) {
        if (std::string(argv[i]) == "-v") {
            if (i + 1 < argc) {
                video_files.push_back(argv[i+1]);
                i++;
            } else {
                std::cerr << "Error: -v requires a video file path." << std::endl;
                return 1;
            }
        } else {
            std::cerr << "Error: Unknown argument " << argv[i] << std::endl;
            return 1;
        }
    }

    if (video_files.empty()) {
        std::cerr << "Error: No video files provided. Use -v <video_path>." << std::endl;
        return 1;
    }

    App app(video_files);
    app.run_stitching();
    return 0;
}