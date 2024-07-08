#ifndef ORPETMW_ORPE_HPP
#define ORPETMW_ORPE_HPP


#include <opencv2/opencv.hpp>

#include "Datastruct.h"
#include "LedClass.h"


namespace ORPETMW {

/**
 * @brief Adds a function that will recieve the image from ORPE. The function will be called when a new image is available.
 * @note thread safe. Receivers must be fast to avoid blocking the ORPE thread.
 * @param receiver The function that will recieve the image.
*/
void addImageReceiver(std::function<void(const cv::Mat)> receiver);

/**
 * @brief Adds a function that will recieve the telemetry from ORPE. The function will be called when a new telemetry is available.
 * @note thread safe.
 * @param receiver The function that will recieve the telemetry.
*/
void addPoseReceiver(std::function<void(const OrpeTelemetry&, const std::vector<LED>&)> receiver);

/**
 * @brief Get the current state of ORPE. Thread safe.
 * @return The current state of ORPE.
*/
ORPEState_t getORPEState();

/**
 * @brief starts ORPE and will return immediately. ORPE will run in the background in a separate thread.
*/
void orpeRun();

/**
 * @brief Shutdown ORPE. Will stop ORPE and clear all receivers.
*/
void shutdownORPE();

}

#endif