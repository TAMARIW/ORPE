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
#include "debug_recording.hpp"
#include "orpe.hpp"


using namespace cv;
using namespace std;
using namespace ORPE;


/**
 * @brief This will loop through the data link system. It will access the other systems.
*/
void dataLinkLoop() {

}




int main(int argc, char **argv) {

    //Add image receivers.
    ORPETMW::addImageReceiver(ORPETMW::debugImageReceiver);
    ORPETMW::addPoseReceiver(ORPETMW::debugPoseReceiver);

    // Initialise video recording.
    ORPETMW::initVideoRecording();

    // Start the ORPE system.
    ORPETMW::orpeRun();

    // Wait for the ORPE system to finish.
    while (ORPETMW::getORPEState() == ORPETMW::ORPE_STATE_RUNNING) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    // Stop video recording to make video file readable.
    ORPETMW::deinitVideoRecording();

    printf("ORPE is done!\n");
    
}

