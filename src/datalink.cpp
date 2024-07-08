
/**
 * This is used to establish a connection with the tamariw datalink process.
*/

#include <iostream>
#include <thread>
#include <atomic>

#include "Datastruct.h"

#include "udp_ipc.hpp"

#include "definitions.hpp"
#include "datalink.hpp"


namespace ORPETMW {


UdpIpc<ORPECommand> commandIpc;
UdpIpc<OrpeTelemetry> telemetryIpc;
UdpIpc<ORPEState_t> stateIpc;

// mutex to protect the telemetry IPC.
std::mutex telemetryIpcMutex;

// mutex to protect the state telemetry IPC.
std::mutex stateIpcMutex;

// The list of telecommand receivers.
std::vector<std::function<void(const ORPECommand&)>> telecommandReceivers;
std::mutex telecommandReceiversMutex;

// The thread that will receive the telecommands from the datalink.
std::thread datalinkThread;


/**
 * @brief This is the thread that will receive the telecommands from the datalink.
*/
void datalinkThreadFunc() {

    ORPECommand command;

    while (true) {
        
        // Check if there is any data to receive. If there is, pass it to the telecommand receivers.
        if (commandIpc.receiveData(command)) {

            std::lock_guard<std::mutex> lock(telecommandReceiversMutex);

            for (auto& receiver : telecommandReceivers) {
                receiver(command);
            }

        }

        //Save cpu resources by limiting to 10ms interval
        std::this_thread::sleep_for(std::chrono::milliseconds(DATALINK_REFRESH_INTERVAL_MS));

    }

}


/**
 * @brief This function initializes the datalink.
*/
void initDatalink() {

    commandIpc.init(DATALINK_ORPETELECOMMAND_CHANNEL, NETWORK_WIDE_DATALINK);
    telemetryIpc.init(DATALINK_ORPETELEMETRY_CHANNEL, NETWORK_WIDE_DATALINK);
    stateIpc.init(DATALINK_ORPESTATE_CHANNEL, NETWORK_WIDE_DATALINK);

    datalinkThread = std::thread(datalinkThreadFunc);
    datalinkThread.detach(); //Detacxh so it runs in the background

}

/**
 * @brief Adds a function that will recieve telecommands from the datalink. 
 * @param receiver The function that will recieve the telecommands.
*/
void addDatalinkCommandReceiver(std::function<void(const ORPECommand&)> receiver) {

    std::lock_guard<std::mutex> lock(telecommandReceiversMutex);
    telecommandReceivers.push_back(receiver);

}

/**
 * @brief This function receives the telemetry data and sends it using udp to the datalink.
 * @param telemetry The telemetry data to send to the datalink.
*/
void datalinkTelemetryReceiver(const OrpeTelemetry& telemetry, const std::vector<LED>& points) {

    std::lock_guard<std::mutex> lock(telemetryIpcMutex);
    telemetryIpc.sendData(telemetry);

}

/**
 * @brief Sends the given state value over datalink
 * @param state State value to send over datalink.
 */
void datalinkSendORPEState(ORPEState_t state) {

    std::lock_guard<std::mutex> lock(stateIpcMutex);
    stateIpc.sendData(state);

}


}
