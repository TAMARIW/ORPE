#include <iostream>
#include <atomic>

#include <lccv.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/videoio.hpp>

#include "NOW.h"
#include "PoseEstimator.h"
#include "Datastruct.h"

#include "definitions.hpp"

#include "datalink.hpp"
#include "orpe.hpp"



namespace ORPETMW {

// Shutdown ORPE if set to true.
std::atomic<bool> shutdownORPEFlag = false;
std::mutex shutdownORPEFlagMutex;

// The current state of ORPE.
std::atomic<ORPEState_t> orpeState = ORPEState_t::ORPEState_Idle;

// The list of image receivers.
std::vector<std::function<void(const cv::Mat)>> imageReceivers;
std::mutex imageReceiversMutex;

// The list of pose receivers.
std::vector<std::function<void(const OrpeTelemetry&, const std::vector<LED>&)>> poseReceivers;
std::mutex poseReceiversMutex;


/**
 * @brief Adds a function that will recieve the image from ORPE. The function will be called when a new image is available. Cleared when ORPE is stopped.
 * @note thread safe. Receivers must be fast to avoid blocking the ORPE thread.
 * @param receiver The function that will recieve the image.
*/
void addImageReceiver(std::function<void(const cv::Mat)> receiver) {
    std::lock_guard<std::mutex> lock(imageReceiversMutex);
    imageReceivers.push_back(receiver);
}

/**
 * @brief Adds a function that will recieve the telemetry from ORPE. The function will be called when a new telemetry is available. Cleared when ORPE is stopped.
 * @note thread safe.
 * @param receiver The function that will recieve the telemetry.
*/
void addPoseReceiver(std::function<void(const OrpeTelemetry&, const std::vector<LED>&)> receiver) {
    std::lock_guard<std::mutex> lock(poseReceiversMutex);
    poseReceivers.push_back(receiver);
}

/**
 * @brief Shutdown ORPE. Will stop ORPE and clear all receivers.
*/
void shutdownORPE() {
    std::lock_guard<std::mutex> lock(shutdownORPEFlagMutex);
    shutdownORPEFlag = true;
}

/**
 * @brief Get the current state of ORPE. Thread safe.
 * @return The current state of ORPE.
*/
ORPEState_t getORPEState() {
    return orpeState;
}



/**
 * @brief The main ORPE thread function. This is where ORPE runs.
*/
void orpeThreadFunction() {

    // Set the state to running
    orpeState = ORPEState_t::ORPEState_Running;

    // Set the shutdown flag to false
    {
        std::lock_guard<std::mutex> lock(shutdownORPEFlagMutex);
        shutdownORPEFlag = false;
    }
    
    // Initialize ORPE
    printf("Starting ORPE.\n");
    PoseEstimator estimator(modelPoints_g, cameraMatrix_g, cameraDistorsionMatrix_g);

    // Initialize the camera
    printf("Camera settings: %d x %d, %d fps, SS: %d, ISO: %f, EV: %f\n", INPUT_WIDTH, INPUT_HEIGHT, FPS, CAMERA_SS, CAMERA_ISO, CAMERA_EV);
    lccv::PiCamera cam;
    cam.options->video_width=INPUT_WIDTH;
    cam.options->video_height=INPUT_HEIGHT;
    cam.options->framerate=FPS;
    cam.options->verbose=true;
    cam.options->shutter=CAMERA_SS;
    cam.options->gain=CAMERA_ISO;
    cam.options->ev=CAMERA_EV;

    // Start the camera
    cam.startVideo();

    // Get the start time
    int64_t runBegin = ORPE::NOW();
    
    // Main loop
    printf("ORPE loop starting.\n");
    while (orpeState == ORPEState_t::ORPEState_Running) {
        int64_t time_ms = ORPE::NOW();
        printf("Orpe loop. Time: %.3f\n", float(time_ms)/1000000);

        // Get the image
        cv::Mat image;
        if (!cam.getVideoFrame(image, 1000)) { // This will synchronise orpe to the camera framerate.
            printf("Failed to read image! Exiting!\n");
            orpeState = ORPEState_t::ORPEState_CamFailed;
            break;
        }

#ifdef RUN_ESTIMATOR
        // Give ORPE the image first
        estimator.giveEstimatorNextImage(image);

        // Run the estimator in a different thread so we can parallelize spreading image and running estimator
        std::thread estimatorThread([&estimator](){
            estimator.estimatePose();
        });
#endif


        // Call the image receivers
        {
            std::lock_guard<std::mutex> lock(imageReceiversMutex);
            for (auto receiver : imageReceivers) {
                receiver(image);
            }
        }

#ifdef RUN_ESTIMATOR
        // Wait for the estimator to finish
        estimatorThread.join();
#endif

        // Retrieve the pose and points
        OrpeTelemetry telemetry;
        bool valid = estimator.getPoseEstimation(telemetry);
        auto points = estimator.getCurrentPoints();

        // Call the pose receivers
        {
            std::lock_guard<std::mutex> lock(poseReceiversMutex);
            for (auto receiver : poseReceivers) {
                receiver(telemetry, points);
            }
        }

#ifdef TIME_LIMIT_SECONDS
        // Check if the time limit has been reached
        if (time_ms - runBegin > TIME_LIMIT_SECONDS*1000000) {
            orpeState = ORPEState_t::ORPEState_Timeout;
            printf("ORPE time out after %.3f s. This is a setting!\n", (float(time_ms - runBegin)/1000000));
            break;
        }
#endif

        // Check if ORPE should be shut down
        if (shutdownORPEFlag) {
            orpeState = ORPEState_t::ORPEState_Stopped;
            printf("ORPE shutdown by command.\n");
            break;
        }


    }

    // Stop the camera and clean up
    cam.stopVideo();
    cv::destroyAllWindows();

    printf("ORPE stopped. State: %d\n", int(orpeState));
    datalinkSendORPEState(orpeState);

}


void orpeRun() {

    // Check if ORPE is already running. We dont want to start it twice.
    if (orpeState == ORPEState_t::ORPEState_Running) {
        printf("ORPE is already running!\n");
        return;
    }

    // To make sure state is set
    orpeState = ORPEState_t::ORPEState_Running;

    // Start the ORPE thread and detach it so it runs in the background.
    std::thread orpeThread(orpeThreadFunction);
    orpeThread.detach();

}

}
