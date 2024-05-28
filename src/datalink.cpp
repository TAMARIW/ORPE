
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

// mutex to protect the telemetry IPC.
std::mutex telemetryIpcMutex;

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

    commandIpc.init(DATALINK_ORPETELECOMMAND_CHANNEL);
    telemetryIpc.init(DATALINK_ORPETELEMETRY_CHANNEL);

    datalinkThread = std::thread(datalinkThreadFunc);

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
void datalinkTelemetryReceiver(const OrpeTelemetry& telemetry);

    std::lock_guard<std::mutex> lock(telemetryIpcMutex);
    telemetryIpc.sendData(telemetry);

}
