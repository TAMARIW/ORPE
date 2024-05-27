#include <iostream>
#include <atomic>

#include <lccv.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/videoio.hpp>

#include "NOW.h"
#include "PoseEstimator.h"
#include "Datastruct.h"

#include "definitions.hpp"

#include "orpe.hpp"



namespace ORPETMW {

// Shutdown ORPE if set to true.
std::atomic<bool> shutdownORPEFlag = false;
std::mutex shutdownORPEFlagMutex;

// The current state of ORPE.
std::atomic<ORPEState> orpeState = ORPE_STATE_IDLE;

// The list of image recievers.
std::vector<std::function<void(const cv::Mat)>> imageRecievers;
std::mutex imageRecieversMutex;

// The list of pose recievers.
std::vector<std::function<void(const OrpeTelemetry&, const std::vector<LED>&)>> poseRecievers;
std::mutex poseRecieversMutex;


/**
 * @brief Adds a function that will recieve the image from ORPE. The function will be called when a new image is available. Cleared when ORPE is stopped.
 * @note thread safe. Recievers must be fast to avoid blocking the ORPE thread.
 * @param reciever The function that will recieve the image.
*/
void addImageReciever(std::function<void(const cv::Mat)> reciever) {
    std::lock_guard<std::mutex> lock(imageRecieversMutex);
    imageRecievers.push_back(reciever);
}

/**
 * @brief Adds a function that will recieve the telemetry from ORPE. The function will be called when a new telemetry is available. Cleared when ORPE is stopped.
 * @note thread safe.
 * @param reciever The function that will recieve the telemetry.
*/
void addPoseReciever(std::function<void(const OrpeTelemetry&, const std::vector<LED>&)> reciever) {
    std::lock_guard<std::mutex> lock(poseRecieversMutex);
    poseRecievers.push_back(reciever);
}

/**
 * @brief Shutdown ORPE. Will stop ORPE and clear all recievers.
*/
void shutdownORPE() {
    std::lock_guard<std::mutex> lock(shutdownORPEFlagMutex);
    shutdownORPEFlag = true;
}

/**
 * @brief Get the current state of ORPE. Thread safe.
 * @return The current state of ORPE.
*/
ORPEState getORPEState() {
    return orpeState;
}



/**
 * @brief The main ORPE thread function. This will run the ORPE in a separate thread.
*/
void orpeThreadFunction() {

    // Set the state to running
    orpeState = ORPE_STATE_RUNNING;

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
    while (orpeState == ORPE_STATE_RUNNING) {
        int64_t time_ms = ORPE::NOW();
        printf("Orpe loop. Time: %ld\n", time_ms);

        // Get the image
        cv::Mat image;
        if (!cam.getVideoFrame(image, 1000)) { // This will synchronise orpe to the camera framerate.
            printf("Failed to read image! Exiting!\n");
            orpeState = ORPE_STATE_CAMFAILED;
            break;
        }

        // Give ORPE the image first
        estimator.giveEstimatorNextImage(image);

        // Run the estimator in a different thread
        std::thread estimatorThread([&estimator](){
            estimator.estimatePose();
        });

        // Call the image recievers
        {
            std::lock_guard<std::mutex> lock(imageRecieversMutex);
            for (auto reciever : imageRecievers) {
                reciever(image);
            }
        }

        // Wait for the estimator to finish
        estimatorThread.join();

        // Retrieve the pose and points
        OrpeTelemetry telemetry;
        bool valid = estimator.getPoseEstimation(telemetry);
        auto points = estimator.getCurrentPoints();

        // Call the pose recievers
        {
            std::lock_guard<std::mutex> lock(poseRecieversMutex);
            for (auto reciever : poseRecievers) {
                reciever(telemetry, points);
            }
        }

        // Check if the time limit has been reached
        if (time_ms - runBegin > TIME_LIMIT_SECONDS*1000000) {
            orpeState = ORPE_STATE_TIMEOUT;\
            printf("ORPE time out after %ld s. This is a setting!\n", ((time_ms - runBegin)/1000000));
            break;
        }

        // Check if ORPE should be shut down
        if (shutdownORPEFlag) {
            orpeState = ORPE_STATE_SHUTDOWN;
            printf("ORPE shutdown by command.\n");
            break;
        }


    }

    // Stop the camera and clean up
    cam.stopVideo();
    cv::destroyAllWindows();

    // Clear the recievers
    {
        std::lock_guard<std::mutex> lock(imageRecieversMutex);
        imageRecievers.clear();
    }
    {
        std::lock_guard<std::mutex> lock(poseRecieversMutex);
        poseRecievers.clear();
    }

    printf("ORPE stopped. State: %d\n", int(orpeState));

}


void orpeRun() {

    // Check if ORPE is already running. We dont want to start it twice.
    if (orpeState == ORPE_STATE_RUNNING) {
        printf("ORPE is already running!\n");
        return;
    }

    // Set the state to running
    orpeState = ORPE_STATE_RUNNING;

    // Start the ORPE thread and detach it so it runs in the background.
    std::thread orpeThread(orpeThreadFunction);
    orpeThread.detach();

}

}
