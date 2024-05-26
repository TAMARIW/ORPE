#ifndef ORPETMW_ORPE_HPP
#define ORPETMW_ORPE_HPP


#include <opencv2/opencv.hpp>

#include "Datastruct.h"
#include "LedClass.h"


namespace ORPETMW {

/**
 * Shows the state of ORPE and can also be used to control ORPE.
*/
enum ORPEState {
    // ORPE is not running.
    ORPE_STATE_IDLE,       
    // ORPE is running. 
    ORPE_STATE_RUNNING,     
    // ORPE timed out due to defined time limit.
    ORPE_STATE_TIMEOUT,     
    // ORPE is shut down by command.
    ORPE_STATE_SHUTDOWN,
    // ORPE failed to read from camera.
    ORPE_STATE_CAMFAILED    
};

/**
 * @brief Adds a function that will recieve the image from ORPE. The function will be called when a new image is available. Cleared when ORPE is stopped.
 * @note thread safe. Recievers must be fast to avoid blocking the ORPE thread.
 * @param reciever The function that will recieve the image.
*/
void addImageReciever(std::function<void(const cv::Mat)> reciever);

/**
 * @brief Adds a function that will recieve the telemetry from ORPE. The function will be called when a new telemetry is available. Cleared when ORPE is stopped.
 * @note thread safe.
 * @param reciever The function that will recieve the telemetry.
*/
void addPoseReciever(std::function<void(const OrpeTelemetry&, const std::vector<LED>&)> reciever);

/**
 * @brief Shutdown ORPE. Will stop ORPE and clear all recievers.
*/
void shutdownORPE();

/**
 * @brief Get the current state of ORPE. Thread safe.
 * @return The current state of ORPE.
*/
ORPEState getORPEState();

/**
 * @brief starts ORPE and will return immediately. ORPE will run in the background in a separate thread.
*/
void orpeRun();

}

#endif