#ifndef DATALINK_H
#define DATALINK_H


#include "rodos.h"

#include <stdint.h>

#include "gateway/router.h"
#include "gateway/gateway.h"

#include "../include/Datastruct.h"

namespace RODOS {

class UartRouter : public Router {
public:
    UartRouter(bool forward_old_topicreports = false,
        Gateway *gateway1 = nullptr, Gateway *gateway2 = nullptr,
        Gateway *gateway3 = nullptr, Gateway *gateway4 = nullptr)
        : Router(forward_old_topicreports, gateway1, gateway2, gateway3, gateway4) {}

    void routeMsg(NetworkMessage& msg, uint32_t linkid) override;
};

}


extern RODOS::Topic<OrpePoseEst> orpePoseEstTopic;

extern RODOS::Topic<bool> orpePowerCommandTopic;
extern RODOS::CommBuffer<bool> orpePowerCommandBuf;

extern RODOS::Topic<float> orpeTestingTopic;

#endif
