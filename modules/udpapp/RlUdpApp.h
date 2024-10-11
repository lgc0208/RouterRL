/*
 * @Author       : LIN Guocheng
 * @Date         : 2024-05-14 17:42:46
 * @LastEditors  : LIN Guocheng
 * @LastEditTime : 2024-09-25 09:31:25
 * @FilePath     : /root/RouterRL/modules/udpapp/RlUdpApp.h
 * @Description  : An Udp packet-sending application for RouterRL.
 */
#ifndef __INET_RlUdpApp_H
#define __INET_RlUdpApp_H
#include <string>
#include "inet/common/INETDefs.h"
#include "inet/applications/base/ApplicationBase.h"
#include "inet/transportlayer/contract/udp/UdpSocket.h"
#include "inet/networklayer/ipv4/RlBasicRoutingTable.h"

using namespace std;

namespace inet
{

/**
 * UDP application. See NED for more info.
 */
class INET_API RlUdpApp : public ApplicationBase, public UdpSocket::ICallback
{
protected:
    enum SelfMsgKinds { START = 1, SEND, STOP, STEP_END };

    // parameters
    vector<string> destAddressStr;
    int localPort = -1, destPort = -1;
    simtime_t startTime;
    simtime_t stopTime;
    bool dontFragment = false;
    const char *packetName = nullptr;

    int nodeNum;                  // Number of nodes in the network topology
    int sendPacketId = 0;         // ID of the packet being sent
    const char *initRoutingTable; // Initial probabilistic routing table
    const char *topoTable;        // Initial topology information, used for split ratios
    int stepTime;                 // Duration of each step
    int zmqPort;                  // ZMQ port
    int messageLength;            // Packet length
    double flowRate;              // Traffic intensity
    double sendInterval;          // Packet sending interval
    int ttl;                      // TTL setting
    int totalStep;                // Total number of simulation steps
    double overTime;              // Timeout
    int returnModeId;             // Simulation mode
    string routingMode;           // Routing mode
    int randDst;                  // Randomly selected destination node
    RlBasicRoutingTable *routingTable;

    UdpSocket socket;
    cMessage *selfMsg = nullptr;
    cMessage *overtimeSelfMsg = nullptr;

    // statistics
    int numSent = 0;     // Number of packets sent
    int numReceived = 0; // Number of packets received

protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void handleMessageWhenUp(cMessage *msg) override;
    virtual void finish() override;
    virtual void refreshDisplay() const override;

    // chooses random destination address
    virtual void sendPacket();
    virtual void processPacket(Packet *msg);
    virtual void setSocketOptions();

    virtual void processStart();
    virtual void processSend();
    virtual void processStop();

    virtual void handleStartOperation(LifecycleOperation *operation) override;
    virtual void handleStopOperation(LifecycleOperation *operation) override;
    virtual void handleCrashOperation(LifecycleOperation *operation) override;

    virtual void socketDataArrived(UdpSocket *socket, Packet *packet) override;
    virtual void socketErrorArrived(UdpSocket *socket, Indication *indication) override;
    virtual void socketClosed(UdpSocket *socket) override;

    int getDstNode();

public:
    RlUdpApp() {}
    ~RlUdpApp();
    simtime_t oTime = 0;
    simtime_t timerStep = 0;
    int stepNum = 0; // Current step number
};

} // namespace inet

#endif // ifndef __INET_RlUdpApp_H