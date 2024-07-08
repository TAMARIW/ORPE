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

    printf("Received cmd: ");

    switch (cmd.command)
    {
    case ORPECommandType_Start:

        printf("ORPE Start\n");
        
        if (ORPETMW::getORPEState() != ORPEState_t::ORPEState_Running) {
            ORPETMW::orpeRun(); //Starts up orpe.
        }

        break;

    case ORPECommandType_Stop:

        printf("ORPE Stop\n");
        
        ORPETMW::shutdownORPE();

        break;

    case ORPECommandType_TakeImage:

        printf("ORPE Take image\n");
        
        if (ORPETMW::getORPEState() == ORPEState_t::ORPEState_Running) {
            //Code to save raw image and send over datalink
        }

        break;

    case ORPECommandType_TakeImageData:

        printf("ORPE Take image with data\n");
        
        if (ORPETMW::getORPEState() == ORPEState_t::ORPEState_Running) {
            //Code to save debug image and send over datalink
        }

        break;

    case ORPECommandType_Shutdown:

        printf("ORPE shutdown!\n");

        orpeShutdown = true;

        break;
    
    default:
        printf("Unknown! (%d)\n", int(cmd.command));
        break;
    }

}



int main(int argc, char **argv) {

    //ORPE initialisation.

    // Make sure we do not immediately leave
    orpeShutdown = false;

    //Add image receivers.
    ORPETMW::addImageReceiver(ORPETMW::debugImageReceiver);
    ORPETMW::addPoseReceiver(ORPETMW::debugPoseReceiver);
    ORPETMW::addPoseReceiver(ORPETMW::datalinkTelemetryReceiver);

    //Add telecommand receivers.
    ORPETMW::addDatalinkCommandReceiver(ORPETMW::datalinkCommandReceiver);

    // Initialise video recording. This is a debug feature and is usually disabled in definitions file.
    ORPETMW::initVideoRecording();
    // Initialise the datalink.
    ORPETMW::initDatalink();
    

    //ORPE process runtime logic
    while (!orpeShutdown) {

        ORPETMW::datalinkSendORPEState(ORPETMW::getORPEState()); //Send the state periodically
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

    }


    //ORPE process shutdown logic.

    // At this point ORPE process is shutting down.
    ORPETMW::shutdownORPE();
    // Wait for the ORPE system to finish.
    while (ORPETMW::getORPEState() == ORPEState_t::ORPEState_Running) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    // Stop video recording to make video file readable.
    ORPETMW::deinitVideoRecording();

    printf("ORPE is shutting down!\n");
    
}

