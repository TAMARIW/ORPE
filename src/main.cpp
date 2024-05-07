#include <iostream>
#include <chrono>
#include <thread>
#include <filesystem>

#include <lccv.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/videoio.hpp>

#include "PoseEstimator.h"

//Below are settings for debugging and runtime
#define RUN_ESTIMATOR
#define CAMERA_ISO -0.1
#define CAMERA_EV  0
#define CAMERA_SS  1 //Shutter speed in microseconds
#define INPUT_WIDTH 1920
#define INPUT_HEIGHT 1080
//#define DESYNC_RESET_CAM //Resets the camera if a desync is detected by the estimator to attempt to fix the issue.
//#define SHOW_DEBUG_IMAGE
//#define WRITE_DEBUG_VIDEO
//define RUN_AT_VIDEO_SPEED
//#define WRITE_DEBUG_IMAGE
#define PRINT_LEDS
#define PRINT_EST
#define TIME_LIMIT 60
#define FORCE_ESTIMATION_START
#define RUN_ONCE
#define FPS 10

#define OUTPUT_WIDTH 1280
#define OUTPUT_HEIGHT 720

#define USE12LED //Use new 12 LED pattern

using namespace cv;
using namespace std;
using namespace ORPE;


vector<LED> modelPoints = {
    //Below are the points used for 12 LED pattern
#ifdef USE12LED
    LED(cv::Point3f(-153, 37.8, 0), 100),
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


Mat cameraMatrix = (cv::Mat_<float>(3, 3) << 1.716e3/1280, 0, 5.9937e2/1280, 0, 1.719e3/720, 3.9096e2/720, 0, 0, 1); // 720p
Mat cameraDistorsionMatrix = (cv::Mat_<float>(5, 1) << 2.3672e-1, -1.2357, -2.7905e-3, -6.1804e-3, 6.5872); //720p

Mat imageWrite;


/// @brief Writes the global imageWrite mat. Used for multithreading
/// @param capture
void imageWriteFunc(cv::VideoWriter &writer)
{

    writer.write(imageWrite);
}

/**
 * @brief Runs the PoseEstimator with a name. If output video is enabled then the output video is given the videoFile parameter as name.
 *
 * @param videoFile
 */
void runEstimatorWithName(std::string videoFile)
{
    videoFile = "SS500";
    //getBuildInformation();
    // Project file path
    std::string directoryPath = __FILE__; //"/home/pi/ORPERodos/ORPE"; // ########### THIS MUST BE CHANGED. Path upto the project home.
    directoryPath = directoryPath.substr(0, directoryPath.find_last_of("/\\"));
    directoryPath = directoryPath.substr(0, directoryPath.find_last_of("/\\"));

    cout << "Directory path: " << directoryPath << endl;
    //while(1);
    PoseEstimator estimator(modelPoints, cameraMatrix, cameraDistorsionMatrix);
    // DataLogger logger(directoryPath, videoFile);
    //while(1);
    // cout << "Opening input video: " << (directoryPath + "/data/Videos/" + videoFile + ".mp4") << endl;
    //  cv::VideoCapture capture(directoryPath + "/data/Videos/" + videoFile + ".mp4");

    //std::string ss = "nvarguscamerasrc !  video/x-raw(memory:NVMM), width=1920, height=1080, format=NV12, framerate=10/1 ! nvvidconv flip-method=2 ! video/x-raw, width=480, height=680, format=BGRx ! videoconvert ! video/x-raw, format=BGR ! appsink";

    //lccv::PiCamera capture;
    //capture.set(CAP_PROP_FORMAT, CV_8UC1 );

    //cv::VideoCapture capture(fifoPath);
    //capture.set(CAP_PROP_FRAME_HEIGHT, 960);
    //capture.set(CAP_PROP_FRAME_WIDTH, 1280);
    //capture.set(CAP_PROP_FPS, FPS);
    //capture.set(CAP_PROP_GAIN, CAMERA_ISO); // Set camera ISO to 100
    //capture.set(CAP_PROP_EXPOSURE, CAMERA_SS);
    //while(1);
    
    //while(1);
    cv::Point3f rVec, tVec; // Declared here to keep old values for iterative pnp-solvers
    rVec = tVec = cv::Point3f(1, 1, 1);

    float estTime = 100000;

// int ex = static_cast<int>(capture.get(CAP_PROP_FOURCC));
// cout << "Opening output video" << endl;

#ifdef WRITE_DEBUG_VIDEO
    cv::VideoWriter writer;
    auto output = directoryPath + "\\data\\video\\output.mp4";
    cout << "Estimator output in: " << output << endl;
    writer.open(output, cv::VideoWriter::fourcc('m', 'p', '4', 'v'), FPS, Size(OUTPUT_WIDTH, OUTPUT_HEIGHT));
    if (!writer.isOpened())
    {
        cout << "Failed to open output video!" << endl;
        return;
    }
#endif

    Mat image;
    //thread imageReadThread(imageReadFunc);

    /*auto inputVideo = directoryPath + "\\data\\video\\" + testType + videoFile + ".mp4";
    cout << "Opening input video: " << inputVideo << endl;
    cv::VideoCapture cap(inputVideo);
    if(!cap.isOpened()){
        cout << "Failed to open input video!" << endl;
        return;
    }*/

    lccv::PiCamera cam;
	
    cam.options->video_width=INPUT_WIDTH;
    cam.options->video_height=INPUT_HEIGHT;
    cam.options->framerate=FPS;
    cam.options->verbose=true;
    cam.options->shutter=CAMERA_SS;
    cam.options->gain=CAMERA_ISO;
    cam.options->ev=CAMERA_EV;
    cam.startVideo();

    //if (!capture.getVideoFrame(image,5000))
    //{
    //    cout << "Failed to open input video!" << endl;
        //capture.stopVideo();
        //return;
    //}

#ifdef WRITE_DEBUG_VIDEO
    if (!writer.isOpened())
    {
        cout << "Failed to open output video!" << endl;
        return;
    }
#endif
    //while(1);

#ifdef WRITE_DEBUG_VIDEO
    thread imageWriteThread;
#endif

    int64_t runBegin = NOW();
    uint32_t frameCounter = 0;
	
    bool leaveLoop = false;
    while (!leaveLoop)
    {
	    //continue;
	int64_t time_ms = NOW();
#ifdef RUN_AT_VIDEO_SPEED
        int64_t nextLoop = time_ms + time_ms%(1000/FPS) + 1000/FPS;
        while(NOW() < nextLoop); //Wait to limit FPS.
#endif

        std::cout << "Time: " << time_ms << std::endl;

        //if (imageReadThread.joinable());
        //    imageReadThread.join(); // Wait for thread to finish reading
        //auto failedRead = !cam.getVideoFrame(image, 1000);
	                                   // Get new image
        //imageReadThread = thread(imageReadFunc, std::ref(capture)); // Restart thread;
	    //capture.grab();
	    //capture.retrieve(image);

        // Capture frame-by-frame
        //cap >> image;
        auto failedRead = !cam.getVideoFrame(image, 1000);
        if (failedRead) {
            std::cout << "Failed to read image! Exiting!" << std::endl;
            //leaveLoop = true;
        }

        if (image.empty())
        {
            std::cout << "Image was empty! Stopping" << std::endl;
            continue;
        }

        cv::Mat cameraImageMatrix = (cv::Mat_<float>(3, 3) << 
            cameraMatrix.at<float>(0, 0) * image.cols, 0, cameraMatrix.at<float>(0, 2) * image.cols,
            0, cameraMatrix.at<float>(1, 1) * image.rows, cameraMatrix.at<float>(1, 2) * image.rows,
            0, 0, 1
        );

#ifdef RUN_ESTIMATOR
        estimator.giveEstimatorNextImage(image);
#endif
	//auto st = NOW();
        estimator.estimatePose();
	//auto en = NOW();

	//cout << "EST TIME: " << (en-st) << endl;

        bool estimationGood = estimator.getPoseEstimation(rVec, tVec);
        if (estimationGood)
        {
#ifdef PRINT_EST
	        cout << "             Rot: " << rVec << ", Pos: " << tVec << endl;
#endif
            cv::drawFrameAxes(image, cameraImageMatrix, cameraDistorsionMatrix, cv::Mat(rVec), cv::Mat(tVec), 80);
            frameCounter++;
        }

	//comPort.sendEstimation(tVec.x, tVec.y, tVec.z, rVec.x, rVec.y, rVec.z, frameCounter);

#ifdef WRITE_DEBUG_VIDEO
#ifdef PRINT_LEDS
	for (auto const& p: estimator.getCurrentPoints()) {
            p.drawLED(image);
        }
#endif
#endif

#ifdef PRINT_LEDS
	cout << "Current LEDs: " << endl;
	for (auto const& p: estimator.getCurrentPoints()) {
	    cout << " - " << p.getId();
	    if (p.getId() > 0) cout << " Identified";
            if (p.isCoding()) cout << " Coding...";
	    cout << endl;
	}
#endif

        // cout << tVec << endl;

        // logger.logData(cv::Mat(rVec), cv::Mat(tVec), estimationGood, estimator.getCurrentFrameNumber(), poseEstTime);

        //uint32_t loopTime = std::chrono::duration_cast<std::chrono::microseconds>(loopEndTimestamp - loopBeginTimestamp).count();

#ifdef WRITE_DEBUG_VIDEO
        //std::ostringstream text;
        //text << "Loop time: " << (loopTime / 1000) << "ms. Estimation time avg: " << float(uint32_t(estTime / 10)) / 100 << "ms";

        //cv::putText(image, text.str(), cv::Point(10, 30), cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(255, 255, 255));

        cv::resize(image, image, cv::Size(OUTPUT_WIDTH, OUTPUT_HEIGHT));

        if (imageWriteThread.joinable())
            imageWriteThread.join();
        image.copyTo(imageWrite);
        imageWriteThread = thread(imageWriteFunc, std::ref(writer));
#endif

#ifdef WRITE_DEBUG_IMAGE
        cv::imwrite(directoryPath + "/data/video/outputs/" + videoFile + ".jpg", image);
#endif

#ifdef SHOW_DEBUG_IMAGE
        cv::imshow("Estimation", image);
#endif

        //std::cout << "Estimation Time: " << float(uint32_t(estTime / 10)) / 100.0f << std::endl;

        // int32_t waitTime = int32_t(1000/videoFPS) - std::chrono::duration_cast<std::chrono::microseconds>(chrono::steady_clock::now() - lastFrameTimestamp).count()/1000;
        // lastFrameTimestamp = chrono::steady_clock::now();
        // if (videoFPS < 0) waitTime = 1;
        // if (waitTime < 1) waitTime = 1;

        // if (waitKey(waitTime) == 27) {
        //     std::cout << "User Exiting program" << std::endl;
        //    break; //Wait long enough for next frame to be timed correctly. If ESC pressed, then exit loop.
        //}

        //comPort.update();

        /*if (comPort.powerCommandIsNew())
        {

            bool powerCommand;
            comPort.getPowerCommand(powerCommand);

            if (!powerCommand)
                break; // Turn off command.
        }*/

#ifdef DESYNC_RESET_CAM
        // If the estimator detected a desync, then stop the camera and restart it.
        if (estimator.isDesynced())
        {
            std::cout << "Desync detected! Resetting camera!" << std::endl;
            cam.stopVideo();
            //usleep(1000000);
            //sleep(1);
            cam.startVideo();
        }
#endif

	//orpeTestingTopic.publish(RODOS::SECONDS_NOW());
	//cout << "Time sec: " << RODOS::SECONDS_NOW() << endl;

#ifdef TIME_LIMIT

        if (NOW() - runBegin > 1000 * TIME_LIMIT)
            break;

#endif
    }

    std::cout << "Estimator stopping!" << std::endl;
	
    //imReadMut.lock();
    //killThreads = true;
    //imReadMut.unlock();
    //cam.stopVideo();

#ifdef WRITE_DEBUG_VIDEO
    if (imageWriteThread.joinable())
        imageWriteThread.join();
#endif

    //capture.stopVideo();
    cam.stopVideo();

#ifdef WRITE_DEBUG_VIDEO
    writer.release();
#endif

    cv::destroyAllWindows();
}


int main(int argc, char **argv) {

    runEstimatorWithName("Forced");
    
}

