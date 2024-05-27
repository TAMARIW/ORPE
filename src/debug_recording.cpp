
/**
 * This class is used for creating and printing debugging information= as with also recording the information and images. 
 * This is not to be used in deployment! Recording is detrimental to performance and should only be used for debugging.
*/

#include <iostream>
#include <filesystem>
#include <thread>
#include <atomic>

#include <opencv2/opencv.hpp>
#include <opencv2/videoio.hpp>

#include "Datastruct.h"

#include "definitions.hpp"
#include "debug_recording.hpp"


#define DEBUG_RECORDING //Enable debug recording. This will create video file in data/video folder.
#define DEBUG_PRINTING  //Enable debug printing. This will print debug information to the console.


namespace ORPETMW {


cv::VideoWriter videoWriter;
std::atomic<bool> videoWriterInitialized = false;

cv::Mat imageWrite;
std::thread imageWriteThread;
std::atomic<bool> imageWriteThreadFinished = true;



void initVideoRecording() {

#ifdef DEBUG_RECORDING

    // Check if the video writer is open. If it is then finalize the video.
    if (videoWriterInitialized) {
        printf("Video writer already initialized! Stopping last video and overwritting!");
        deinitVideoRecording();
    }

    // Get the file path to this project. Assuming the executable is in the project build folder.
    std::string directoryPath = __FILE__;
    directoryPath = directoryPath.substr(0, directoryPath.find_last_of("/\\"));
    directoryPath = directoryPath.substr(0, directoryPath.find_last_of("/\\"));

    // Create the video writer.
    auto output = directoryPath + "/data/video/output.mp4"; // Output file path.
    printf("Debug video output in: %s\n", output.c_str());
    videoWriter.open(output, cv::VideoWriter::fourcc('m', 'p', '4', 'v'), FPS, cv::Size(OUTPUT_WIDTH, OUTPUT_HEIGHT));
    
    // Check if the video writer is open.
    if (!videoWriter.isOpened())
    {
        printf("Failed to open output video!\n");
        videoWriterInitialized = false;
        videoWriter.release();
        return;
    }

    videoWriterInitialized = true;

#endif

}


void deinitVideoRecording() {

#ifdef DEBUG_RECORDING

    // Check if the video writer is open.
    if (!videoWriterInitialized) {
        printf("Video writer not initialized!");
        return;
    }

    // Wait for the image write thread to finish.
    if (imageWriteThread.joinable())
        imageWriteThread.join();
    
    // Close the video writer. This will make the video file ready for playback.
    videoWriter.release();

#endif

}


/**
 * @brief This function recieves the images from ORPE and records them to a video file.
 * @note This will do nothing if DEBUG_RECORDING is not defined.
 * @param image The image to record.
 * 
*/
void debugImageReciever(const cv::Mat image) {

#ifdef DEBUG_RECORDING

    if (!videoWriterInitialized) {
        printf("Video writer not initialized!");
        return;
    }

    // Check if the last image write thread is still running.
    if (!imageWriteThreadFinished) {
        printf("Image write thread still running! Skipping frame!");
        return;
    }

    // Copy and resize the image.
    image.copyTo(imageWrite);
    cv::resize(imageWrite, imageWrite, cv::Size(OUTPUT_WIDTH, OUTPUT_HEIGHT));

    // Start the image write thread.
    imageWriteThreadFinished = false;
    imageWriteThread = std::thread([](cv::Mat image) {
        videoWriter.write(image);
        imageWriteThreadFinished = true;
    }, imageWrite);

#endif

}


/**
 * @brief This function recieves the telemetry from ORPE and records them to a file and prints them to the console.
 * @note This will do nothing if DEBUG_PRINTING is not defined.
 * @param telemetry The telemetry to record.
*/
void debugPoseReciever(const OrpeTelemetry& telemetry, const std::vector<LED>& points) {

    // Simply print the telemetry to the console.

#ifdef DEBUG_PRINTING

    // decode the led ID information.
    std::vector<int> ledIDs;
    for (int i = 0; i < 15; i++) {
        ledIDs.push_back((telemetry.ledIDs >> (i * 2)) & 0b00000011);
    }

    //Format to a list containing the led IDs as their index and their values. If 0 then do not print.
    std::string ledIDString = "";
    /*for (int i = 0; i < 15; i++) {
        if (ledIDs[i] != 0) {
            ledIDString += std::to_string(i) + ": " + std::to_string(ledIDs[i]) + "\n\t";
        }
    }*/
    for (const LED& p : points) { //The same as above but with the LED vector.
        ledIDString += " - " + std::to_string(p.getId());
        if (p.getId() > 0) ledIDString += " Identified";
        if (p.isCoding()) ledIDString += " Coding...";
        ledIDString += "\n";
    }

    printf("Telemetry \npos: %f, %f, %f\nrot: %f, %f, %f\nPoints: %d\nIDs: \t %s", telemetry.px, telemetry.py, telemetry.pz, telemetry.ax, telemetry.ay, telemetry.az, telemetry.numPoints, ledIDString.c_str());

#endif

}


}
