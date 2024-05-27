#ifndef ORPETMW_DEFINITIONS_HPP
#define ORPETMW_DEFINITIONS_HPP

#include "opencv2/opencv.hpp"

#include "LedClass.h"


//Below are settings for debugging and runtime
#define RUN_ESTIMATOR
#define CAMERA_ISO -0.1
#define CAMERA_EV  0
#define CAMERA_SS  1 //Shutter speed in microseconds
#define INPUT_WIDTH 1280
#define INPUT_HEIGHT 720
//#define DESYNC_RESET_CAM //Resets the camera if a desync is detected by the estimator to attempt to fix the issue.
//#define SHOW_DEBUG_IMAGE
//#define WRITE_DEBUG_VIDEO
//#define RUN_AT_VIDEO_SPEED
//#define WRITE_DEBUG_IMAGE
#define PRINT_LEDS
#define PRINT_EST
#define TIME_LIMIT_SECONDS 60
#define FORCE_ESTIMATION_START
#define RUN_ONCE
#define FPS 20

#define OUTPUT_WIDTH 1280
#define OUTPUT_HEIGHT 720

#define USE12LED //Use new 12 LED pattern



static std::vector<LED> modelPoints_g = {
    //Below are the points used for 12 LED pattern
#ifdef USE12LED
    LED(cv::Point3f(-153, 37.8, 0), 1),
    LED(cv::Point3f(-154.4, -38.4, 0), 2),
    LED(cv::Point3f(-63, 6, 0), 3),
    LED(cv::Point3f(60.5, 8.5, 0), 4),
    LED(cv::Point3f(153, 37.8, 0), 5),
    LED(cv::Point3f(154.4, -38.4, 0), 6),

    LED(cv::Point3f(-17.9, 11.2, 0), 7),
    LED(cv::Point3f(-19, -8, 0), 8),
    LED(cv::Point3f(-3.9, 7.6, 0), 9),
    LED(cv::Point3f(-0.6, -10.9, 0), 10),
    LED(cv::Point3f(13.3, 11.2, 0), 11),
    LED(cv::Point3f(12.4, -5.6, 0), 12)
#else
    //Below are the points used for 8 LED pattern
    LED(cv::Point3f(0, 0, 0), 0b01011110),
    LED(cv::Point3f(0, 86, 0), 0b10101111),
    LED(cv::Point3f(100, 0, 0), 0b11010111),
    LED(cv::Point3f(100, 86, 0), 0b01011011),
    LED(cv::Point3f(200, 0, 0), 0b11011010),
    LED(cv::Point3f(200, 86, 0), 0b11010000),
    LED(cv::Point3f(307, 0, 0), 0b10010110),
    LED(cv::Point3f(307, 86, 0), 0b11110101)
#endif
};


static cv::Mat cameraMatrix_g = (cv::Mat_<float>(3, 3) << 1.716e3/1280, 0, 5.9937e2/1280, 0, 1.719e3/720, 3.9096e2/720, 0, 0, 1);
static cv::Mat cameraDistorsionMatrix_g = (cv::Mat_<float>(5, 1) << 2.3672e-1, -1.2357, -2.7905e-3, -6.1804e-3, 6.5872);


#endif //ORPETMW_DEFINITIONS_HPP


