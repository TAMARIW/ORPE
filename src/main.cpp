#include <iostream>
#include <chrono>
#include <thread>
#include <filesystem>

#include <lccv.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/videoio.hpp>

#include "udp_ipc.hpp"

#include "NOW.h"
#include "PoseEstimator.h"

#include "definitions.hpp"
#include "datalink.hpp"
#include "debug_recording.hpp"
#include "orpe.hpp"


using namespace cv;
using namespace std;
using namespace ORPE;


//Global variable to control ORPE system state
std::atomic<bool> orpeShutdown = false;


/**
 * @brief This function receives the telecommands from the datalink and controls ORPE. Implementation of function
 * @param cmd The command received from datalink.
*/
void ORPETMW::datalinkCommandReceiver(const ORPECommand& cmd) {

    switch (cmd.command)
    {
    case ORPECommandType_Start:
        
        if (ORPETMW::getORPEState() != ORPETMW::ORPEState::ORPE_STATE_RUNNING) {
            ORPETMW::orpeRun(); //Starts up orpe.
        }

        break;

    case ORPECommandType_Stop:
        
        ORPETMW::shutdownORPE();

        break;

    case ORPECommandType_TakeImage:
        
        if (ORPETMW::getORPEState() == ORPETMW::ORPEState::ORPE_STATE_RUNNING) {
            //Code to trigger raw image capture
        }

        break;

    case ORPECommandType_TakeImageData:
        
        if (ORPETMW::getORPEState() == ORPETMW::ORPEState::ORPE_STATE_RUNNING) {
            //Code to trigger debug image capture
        }

        break;

    case ORPECommandType_Shutdown:

        orpeShutdown = true;

        break;
    
    default:
        break;
    }

}



int main(int argc, char **argv) {

    //ORPE initialisation.

    //Add image receivers.
    ORPETMW::addImageReceiver(ORPETMW::debugImageReceiver);
    ORPETMW::addPoseReceiver(ORPETMW::debugPoseReceiver);
    ORPETMW::addPoseReceiver(ORPETMW::datalinkTelemetryReceiver);

    //Add telecommand receivers.
    ORPETMW::addDatalinkCommandReceiver(ORPETMW::datalinkCommandReceiver);

    // Initialise video recording. This is a debug feature and is usually disabled.
    ORPETMW::initVideoRecording();
    // Initialise the datalink.
    ORPETMW::initDatalink();
    // Make sure we do not immediately leave
    orpeShutdown = false;
    

    //ORPE process runtime logic
    while (!orpeShutdown) {

        datalinkSendORPEState(ORPETMW::getORPEState()); //Send the state periodically
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

    }


    //ORPE process shutdown logic.

    // At this point ORPE process is shutting down.
    ORPETMW::shutdownORPE();
    // Wait for the ORPE system to finish.
    while (ORPETMW::getORPEState() == ORPETMW::ORPE_STATE_RUNNING) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    // Stop video recording to make video file readable.
    ORPETMW::deinitVideoRecording();

    printf("ORPE is shutting down!\n");
    
}

