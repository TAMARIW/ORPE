#ifndef ORPETMW_DEBUGRECORDING_HPP
#define ORPETMW_DEBUGRECORDING_HPP


/**
 * This is used for creating and printing debugging information= as with also recording the information and images. 
 * This is not to be used in deployment! Recording is detrimental to performance and should only be used for debugging.
*/


#include <opencv2/opencv.hpp>

#include "Datastruct.h"


#define DEBUG_RECORDING //Enable debug recording. This will create video file in data/video folder.
#define DEBUG_PRINTING  //Enable debug printing. This will print debug information to the console.


namespace ORPETMW {

/**
 * @brief This function initializes the video recording.
 * @note This will do nothing if DEBUG_RECORDING is not defined.
*/
void initVideoRecording();


/**
 * @brief This function deinitalizes the video recording. This will close the video file making it ready for playback.
 * @note This will do nothing if DEBUG_RECORDING is not defined.
*/
void deinitVideoRecording();

/**
 * @brief This function recieves the images from ORPE and records them to a video file.
 * @note This will do nothing if DEBUG_RECORDING is not defined.
 * @param image The image to record.
 * 
*/
void debugImageReceiver(const cv::Mat image);

/**
 * @brief This function recieves the telemetry from ORPE and records them to a file and prints them to the console.
 * @note This will do nothing if DEBUG_PRINTING is not defined.
 * @param telemetry The telemetry to record.
*/
void debugPoseReceiver(const OrpeTelemetry& telemetry, const std::vector<LED>& points);

}

#endif