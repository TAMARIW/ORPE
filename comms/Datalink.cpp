#include <stddef.h>
#include <iostream>
#include "rodos.h"

#include "gateway/linkinterface.h"
#include "gateway/gateway.h"
#include "timemodel.h"
#include "netmsginfo.h"

#include "../ipc/ipcRODOS.h"

#include "Datalink.h"


//#define PWR_TIMEOUT_EXIT 60*SECONDS //Define this to auto exit program if no power command was received for the defined amount of time

//#define AUTORUN_ESTIMATOR //Uncomment this to have comms autostart estimator in background


// UART setup
static HAL_UART uart(UART_IDX4);
static int init_dummy = uart.init(115200);

// Gateway setup
static LinkinterfaceUART uart_linkinterface(&uart, 115200);
static Gateway uart_gateway(&uart_linkinterface, true);

static UDPInOut udp(-50000);
static LinkinterfaceUDP linkinterface(&udp);
static Gateway udp_gateway(&linkinterface, true);


//Structs
struct Command
{
	int id;
	float fval_1, fval_2, fval_3;
};

struct TelemetryCalibIMU
{
    float gyro_offx, gyro_offy, gyro_offz;          // [rad/s]
    float accel_offx, accel_offy, accel_offz;       // [g]
    float mag_offx, mag_offy, mag_offz;             // [gauss]
};

struct TelemetryControlParams
{
    float speed_P, speed_I, speed_D, speed_lim;
    float pos_P, pos_I, pos_D, pos_lim;
    float vel_P, vel_I, vel_D, vel_lim;
};


//Topics
RODOS::Topic<OrpePoseEst> orpePoseEstTopic(400, "Camera Topic");
RODOS::Topic<bool> orpePowerCommandTopic(401, "OrpePowerCommandTopic");
RODOS::Topic<bool> orpeShutdownTopic(403, "OrpePowerCommandTopic");
RODOS::Topic<Command> tcCmdG(51, "Telecommand Topic");
RODOS::Topic<Command> tcCmdS(50, "Telecommand Topic");

RODOS::Topic<TelemetryCalibIMU> tmCalG(802, "TMCalibIMUG");
RODOS::Topic<TelemetryCalibIMU> tmCalS(42, "TMCalibIMUS");

RODOS::Topic<TelemetryControlParams> tmPrmG(803, "TMCnrtPrm");
RODOS::Topic<TelemetryControlParams> tmPrmS(43, "TMCnrtPrm");

RODOS::Topic<float> orpeTestingTopic(402, "OrpeTesting");

//Buffers
RODOS::CommBuffer<bool> orpePowerCommandBuf;
RODOS::CommBuffer<Command> tcBuf;

RODOS::CommBuffer<TelemetryCalibIMU> tmCBuf;
RODOS::CommBuffer<TelemetryControlParams> tmPBuf;

//Subscribers
RODOS::Subscriber orpePowerCommandSubr(orpePowerCommandTopic, orpePowerCommandBuf, "Orpe Power Command");
RODOS::Subscriber tcSubrG(tcCmdG, tcBuf, "Telecommand subscriber");

RODOS::Subscriber tmCSubrS(tmCalS, tmCBuf, "Telemetry control subr");
RODOS::Subscriber tmPSubrS(tmPrmS, tmPBuf, "Telemetry param subr");

//Router
RODOS::Router gatewayRouter(false, &uart_gateway, &udp_gateway);

// Init before scheduling (dont know why, taken from examples -Max)
class GatewayInitiator : public Initiator
{
    void init()
    {   

        //uart_linkinterface.init();

        // Add Topic to forward
        //uart_gateway.resetTopicsToForward();
        //uart_gateway.addTopicsToForward(&orpePoseEstTopic);
        //uart_gateway.addTopicsToForward(&orpePowerCommandTopic);

        //udp_gateway.resetTopicsToForward();
        //udp_gateway.addTopicsToForward(&orpePoseEstTopic);
        //udp_gateway.addTopicsToForward(&orpePowerCommandTopic);
	
	    //uart_gateway.addTopicsToForward(&orpeTestingTopic);
	    //uart_gateway.addTopicsToForward(&orpeTestingTopic);

        //uart_gateway.addTopicsToForward(&tcCmd);
        //udp_gateway.addTopicsToForward(&tcCmd);

    }
};

//GatewayInitiator gatewayinitiator;


RODOS::Semaphore ipcInitSem;


void prtctIPCInit() {

    ipcInitSem.enter();

    static bool init = false;

    if (!init)
        ipcInit();

    init = true;

    ipcInitSem.leave();


}


class ORPEEstComms : public StaticThread<1000000> {

    void init() override {



    }


    void run() override {

        prtctIPCInit();

        printf("RODOS comms init!\n");
 
#ifdef AUTORUN_ESTIMATOR
        //system("./ESTIMATOR &"); //Start estimator
#endif

        while (1) {

            //sendORPEPowerCommand(powerCommand);
            //powerCommand = !powerCommand;

            OrpePoseEst est;
            if (getORPEEstimation(est)) {
                printf("Est t: %f, %f, %f \n   v: %f, %f, %f \n    c: %d \n    t: %f    v:%d\n", est.px_, est.py_, est.pz_, est.ax_, est.ay_, est.az_, est.frameNum_, SECONDS_NOW(), est.valid_);
                orpePoseEstTopic.publish(est);
            }

            orpePoseEstTopic.publish(est);

            //printf("Active loop. Time %f\n", SECONDS_NOW());

            suspendCallerUntil(NOW() + 10*MILLISECONDS);


        }


    }

};

ORPEEstComms orpeComms;


class ORPEPwrComms : public StaticThread<1000000> {
public:

    RODOS::CommBuffer<bool> buf;
    RODOS::Subscriber subr;

    RODOS::CommBuffer<bool> bufSD;
    RODOS::Subscriber subrSD;

    ORPEPwrComms() : 
        subr(orpePowerCommandTopic, buf),
        subrSD(orpeShutdownTopic, bufSD)
    {}

    void init() override {



    }

    void run() override {

        prtctIPCInit();
        
        bool powerSetting = false;

        int64_t lastPowerCommand = 0;

        while (1) {

            bool pwr;
            if (buf.getOnlyIfNewData(pwr)) {
                powerSetting = pwr;
                lastPowerCommand = NOW();
                printf("New power setting (%d), sending to ORPE...\n", pwr);
            }

            //bool sd;
            //if (buf.getOnlyIfNewData(sd) && sd) {
            //    printf("Got command to shutdown...\n");
            //    suspendCallerUntil(NOW() + 500*MILLISECONDS);
            //    system("sudo shutdown now");
            //}

#ifdef PWR_TIMEOUT_EXIT
            //if (NOW() - lastPowerCommand > PWR_TIMEOUT_EXIT)
            //{
            //    exit(1);
            //}
#endif

            //sendORPEPowerCommand(powerSetting);

            suspendCallerUntil(NOW() + 200*MILLISECONDS);

        }

    }

};

ORPEPwrComms pwrComms;


class ORPETest : public StaticThread<> {
public:

    RODOS::CommBuffer<float> buf;
    RODOS::Subscriber subr;

    ORPETest() : StaticThread("ORPETest", 10), subr(orpeTestingTopic, buf) {}

    void init() override {



    }

    void run() override {

        float time = 0;
        while (1) {

            if (buf.getOnlyIfNewData(time)) {
                printf("Time %f\n", time);
            }
		
            Command c;
            if (tcBuf.getOnlyIfNewData(c)) {
                printf("Command id: %d", c.id);
                tcCmdS.publish(c);
                //tcCmd.publish(c);
            }

	    TelemetryControlParams tmp;
            if (tmPBuf.getOnlyIfNewData(tmp)) {
                printf("Control params %f\n", tmp.speed_P);
                tmPrmG.publish(tmp);
                //tcCmd.publish(c);
            }

	    TelemetryCalibIMU tmc;
            if (tmCBuf.getOnlyIfNewData(tmc)) {
                printf("Calib received %f\n", tmc.mag_offx);
                tmCalG.publish(tmc);
                //tcCmd.publish(c);
            }

            //printf("LOOP\n");
            
            suspendCallerUntil(NOW() + 100*MILLISECONDS);
            //suspendCallerUntil(NOW() + 1*SECONDS);

        }

    }

};

ORPETest testThread;


