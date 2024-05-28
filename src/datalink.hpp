#ifndef ORPETMW_DATALINK_HPP
#define ORPETMW_DATALINK_HPP


/**
 * This is used to establish a connection with the tamariw datalink process.
*/


#include <opencv2/opencv.hpp>

#include "Datastruct.h"


namespace ORPETMW {

/**
 * @brief This function initializes the datalink.
*/
void initDatalink();

/**
 * @brief Adds a function that will recieve telecommands from the datalink. 
 * @param receiver The function that will recieve the telecommands.
*/
void addDatalinkCommandReceiver(std::function<void(const ORPECommand&)> receiver);

/**
 * @brief This function receives the telemetry data and sends it using udp to the datalink.
 * @param telemetry The telemetry data to send to the datalink.
*/
void datalinkTelemetryReceiver(const OrpeTelemetry& telemetry);

}

#endif