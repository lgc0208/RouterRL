/*
 * @Author       : LIN Guocheng
 * @Date         : 2024-09-20 04:05:27
 * @LastEditors  : LIN Guocheng
 * @LastEditTime : 2024-10-09 07:23:07
 * @FilePath     : /root/RouterRL/modules/udpapp/RlUdpApp.cc
 * @Description  : Packet-sending application using UDP protocol.
 */
#include "inet/applications/udpapp/RlUdpApp.h"
#include "inet/applications/base/ApplicationPacket_m.h"
#include "inet/common/TagBase_m.h"
#include "inet/common/TimeTag_m.h"
#include "inet/networklayer/common/FragmentationTag_m.h"
#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/networklayer/ipv4/RlConventionalRoutingTable.h"
#include "inet/networklayer/ipv4/RlProbabilisticRoutingTable.h"
#include "inet/networklayer/ipv4/RlMultipathRoutingTable.h"
#include "inet/networklayer/ipv4/RlPathRoutingTable.h"

namespace inet
{

Define_Module(RlUdpApp);

RlUdpApp::~RlUdpApp()
{
    cancelAndDelete(selfMsg);
}

/**
     * @brief Initializes the probabilistic routing application deployed on each Host, based on the UDP protocol
     *
     * @param stage Initialization stage
     */
void RlUdpApp::initialize(int stage)
{
    ApplicationBase::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        numSent = 0;
        numReceived = 0;
        WATCH(numSent);
        WATCH(numReceived);

        nodeNum = par("nodeNum");
        stepTime = par("stepTime");
        zmqPort = par("zmqPort");
        initRoutingTable = par("initRoutingTable");
        messageLength = par("messageLength");
        flowRate = par("flowRate"); // Unit: Mbps
        sendInterval = 1 / (flowRate * 1000 * 1000 / 8 / messageLength);
        overTime = par("overTime");
        overTime = (overTime == -1) ? INT_MAX : overTime;
        totalStep = par("totalStep");
        string returnMode = par("returnMode").stringValue();
        if (returnMode == "global") {
            returnModeId = 0;
        } else if (returnMode == "distributed") {
            returnModeId = 1;
        } else {
            cout << "Return Mode Error!" << endl;
            return;
        }
        routingMode = par("routingMode").stringValue();
        topoTable = par("topoTable");

        unordered_map<string, function<void()>> initFunctions = {
            {"convention",
             [&]() {
                 RlConventionalRoutingTable::initTable(nodeNum, zmqPort, overTime, totalStep);
             }},
            {"probabilistic",
             [&]() {
                 RlProbabilisticRoutingTable::initTable(nodeNum, initRoutingTable, zmqPort,
                                                        overTime, totalStep, returnModeId);
             }},
            {"multipath",
             [&]() {
                 RlMultipathRoutingTable::initTable(nodeNum, topoTable, initRoutingTable, zmqPort,
                                                    overTime, totalStep, returnModeId);
             }},
            {"singlepath",
             [&]() {
                 RlPathRoutingTable::initTable(nodeNum, topoTable, initRoutingTable, zmqPort,
                                               overTime, totalStep, returnModeId);
             }},
        };

        unordered_map<string, function<RlBasicRoutingTable *()>> getInstanceFunctions = {
            {"convention",
             []() {
                 return static_cast<RlBasicRoutingTable *>(
                     RlConventionalRoutingTable::getInstance());
             }},
            {"probabilistic",
             []() {
                 return static_cast<RlBasicRoutingTable *>(
                     RlProbabilisticRoutingTable::getInstance());
             }},
            {"multipath",
             []() {
                 return static_cast<RlBasicRoutingTable *>(RlMultipathRoutingTable::getInstance());
             }},
            {"singlepath",
             []() {
                 return static_cast<RlBasicRoutingTable *>(RlPathRoutingTable::getInstance());
             }},
        };

        if (initFunctions.count(routingMode)) {
            initFunctions[routingMode]();
            routingTable = getInstanceFunctions[routingMode]();
        }

        localPort = par("localPort");
        destPort = par("destPort");
        startTime = par("startTime");
        stopTime = par("stopTime");
        dontFragment = par("dontFragment");
        if (stopTime >= SIMTIME_ZERO && stopTime < startTime)
            throw cRuntimeError("Invalid startTime/stopTime parameters");
        selfMsg = new cMessage("sendTimer");
        overtimeSelfMsg = new cMessage("overtime");
        randDst = getDstNode();
    }
}

/**
     * @brief Randomly gets the destination node for packet transmission, called before sending the packet
     *
     * @return int ID of the destination node
     */
int RlUdpApp::getDstNode()
{
    // Get the current time in microseconds
    auto now = std::chrono::system_clock::now();
    auto now_us = std::chrono::time_point_cast<std::chrono::microseconds>(now);
    auto value = now_us.time_since_epoch();
    long micros = value.count();
    // Use the microsecond value as the random seed
    std::mt19937 gen(micros);
    // Get the current node, i.e., the source node
    string sender = getParentModule()->getFullName();
    int senderNode = atoi(sender.c_str() + 2);
    int dst = senderNode;
    while (dst == senderNode) {
        std::uniform_int_distribution<int> dist(0, nodeNum - 1);
        dst = dist(gen);
    }
    return dst;
}

/**
     * @brief Ends the current Host's probabilistic routing application
     *
     */
void RlUdpApp::finish()
{
    ApplicationBase::finish();
}

/**
     * @brief Sets the basic information for packet transmission via Socket
     *
     */
void RlUdpApp::setSocketOptions()
{
    int timeToLive = par("timeToLive");
    if (timeToLive != -1)
        socket.setTimeToLive(timeToLive);

    int dscp = par("dscp");
    if (dscp != -1)
        socket.setDscp(dscp);

    int tos = par("tos");
    if (tos != -1)
        socket.setTos(tos);

    const char *multicastInterface = par("multicastInterface");
    if (multicastInterface[0]) {
        IInterfaceTable *ift = getModuleFromPar<IInterfaceTable>(par("interfaceTableModule"), this);
        NetworkInterface *ie = ift->findInterfaceByName(multicastInterface);
        if (!ie)
            throw cRuntimeError("Wrong multicastInterface setting: no interface named \"%s\"",
                                multicastInterface);
        socket.setMulticastOutputInterface(ie->getInterfaceId());
    }

    bool receiveBroadcast = par("receiveBroadcast");
    if (receiveBroadcast)
        socket.setBroadcast(true);

    bool joinLocalMulticastGroups = par("joinLocalMulticastGroups");
    if (joinLocalMulticastGroups) {
        MulticastGroupList mgl =
            getModuleFromPar<IInterfaceTable>(par("interfaceTableModule"), this)
                ->collectMulticastGroups();
        socket.joinLocalMulticastGroups(mgl);
    }
    socket.setCallback(this);
}

/**
     * @brief Sends a packet. Sends a probabilistic routing packet via the socket to a target node as set
     *
     */
void RlUdpApp::sendPacket()
{
    string sender = getParentModule()->getFullName(); // Current node, i.e., the source node
    int senderNode = atoi(sender.c_str() + 2);

    for (int dst = 0; dst < nodeNum; dst++) {
        if (dst == senderNode)
            continue;

        string destName = "H[" + to_string(dst) + "]";
        int sendId = routingTable->getSendId();
        string pkName = routingMode + par("returnMode").stringValue();
        Packet *packet = new Packet(pkName.c_str());
        // Variables carried by the packet include the step, packet ID, source node, and destination node
        packet->addPar("step").setLongValue(stepNum);
        packet->addPar("src").setStringValue(sender.c_str());
        packet->addPar("dst").setStringValue(destName.c_str());
        packet->addPar("id").setLongValue(sendId);
        sendPacketId++;

        if (dontFragment)
            packet->addTag<FragmentationReq>()->setDontFragment(true);
        const auto &payload = makeShared<ApplicationPacket>();

        payload->setChunkLength(B((int)messageLength));
        payload->setSequenceNumber(numSent);
        payload->addTag<CreationTimeTag>()->setCreationTime(simTime());
        packet->insertAtBack(payload);

        L3Address destAddr;
        const char *dn = destName.c_str();
        cStringTokenizer tokenizer(dn);
        const char *token = tokenizer.nextToken();
        L3AddressResolver().tryResolve(token, destAddr);
        emit(packetSentSignal, packet);

        socket.sendTo(packet, destAddr, destPort); // Send packet via socket
        numSent++;
    }
}

/**
     * @brief Starts the probabilistic routing application process
     *
     */
void RlUdpApp::processStart()
{
    socket.setOutputGate(gate("socketOut"));
    const char *localAddress = par("localAddress");
    socket.bind(*localAddress ? L3AddressResolver().resolve(localAddress) : L3Address(), localPort);
    setSocketOptions();

    selfMsg->setKind(SEND);
    processSend();
}

/**
     * @brief Calls the probabilistic routing process to send messages. Sends packet information at a certain interval or sends a stop step instruction based on different sending methods
     *
     */
void RlUdpApp::processSend()
{
    // Only send packets when the current step is less than the totalStep set
    if (stepNum < totalStep) {
        // Initialize the timer
        if (timerStep == 0) {
            timerStep = simTime();
        }
        // Calculate the packet transmission interval
        simtime_t timeC = simTime() - timerStep;
        if (timeC > stepTime) {
            routingTable->recordPktNum(sendPacketId, stepNum);
            routingTable->countNodeEndInStep(stepNum, simTime().dbl());
            routingTable->updateRoutingTable(stepNum, timeC.dbl());
            timerStep = simTime();

            // End the current step, and send a forced end self-message after survivalTime to avoid complete packet loss
            overtimeSelfMsg->setKind(STEP_END);
            if (overtimeSelfMsg->hasPar("step")) {
                overtimeSelfMsg->par("step").setLongValue(stepNum);
            } else {
                overtimeSelfMsg->addPar("step").setLongValue(stepNum);
            }
            scheduleAt(simTime() + overTime, overtimeSelfMsg);

            stepNum++;
            randDst = getDstNode();
            sendPacketId = 0;
        }
        sendPacket();
        // Based on the set traffic intensity and average packet length, get the average packet transmission interval and generate an exponential distribution packet transmission interval
        cMersenneTwister *mt;
        // Traffic generation method: bounded, uniform distribution
        cUniform un = cUniform(mt, 0.9 * sendInterval, 1.1 * sendInterval);
        // cExponential ex = cExponential(mt, sendInterval);
        double interval = un.draw();
        simtime_t d = simTime() + interval;
        // This environment is set to send packets continuously (stopTime=-1), so it only enters the first if
        if (stopTime < SIMTIME_ZERO || d < stopTime) {
            selfMsg->setKind(SEND);
            scheduleAt(d, selfMsg);
        } else {
            selfMsg->setKind(STOP);
            scheduleAt(stopTime, selfMsg);
        }
    }
}

/**
     * @brief Stops the probabilistic routing application process
     *
     */
void RlUdpApp::processStop()
{
    socket.close();
}

/**
     * @brief Handles incoming messages. All received messages are handled, if it's a selfMsg, execute the corresponding flag, if it's a message with itself as the destination node, judge, handle, and count
     *
     * @param msg Message to be processed
     */
void RlUdpApp::handleMessageWhenUp(cMessage *msg)
{
    if (msg->isSelfMessage()) {
        ASSERT(msg == selfMsg || msg == overtimeSelfMsg);
        if (msg == selfMsg) {
            switch (selfMsg->getKind()) {
                case START:
                    processStart();
                    break;

                case SEND:
                    processSend();
                    break;

                case STOP:
                    processStop();
                    break;

                default:
                    throw cRuntimeError("Invalid kind %d in self message", (int)selfMsg->getKind());
            }
        } else if (msg == overtimeSelfMsg) {
            int step = msg->par("step").longValue();
            if (routingTable->stepFinished[step]) {
                return;
            }
            switch (overtimeSelfMsg->getKind()) {
                case STEP_END:
                    routingTable->stepOverJudge(step, simTime().dbl());
                    break;

                default:
                    throw cRuntimeError("Invalid kind %d in overtime self message",
                                        (int)overtimeSelfMsg->getKind());
            }
        }
    } else {
        if (strstr(msg->getFullName(), routingMode.c_str())) {
            socket.processMessage(msg);
        }
    }
}

/**
     * @brief Called when a packet transmitted via socket arrives
     *
     * @param socket Socket information
     * @param packet Packet information
     */
void RlUdpApp::socketDataArrived(UdpSocket *socket, Packet *packet)
{
    // process incoming packet
    processPacket(packet);
}

/**
     * @brief Called when an error occurs during socket transmission
     *
     * @param socket        Socket information
     * @param indication    Error report information
     */
void RlUdpApp::socketErrorArrived(UdpSocket *socket, Indication *indication)
{
    EV_WARN << "Ignoring UDP error report " << indication->getName() << endl;

    delete indication;
}

/**
     * @brief Closes the transmission socket
     *
     * @param socket Socket information to be closed
     */
void RlUdpApp::socketClosed(UdpSocket *socket)
{
    if (operationalState == State::STOPPING_OPERATION)
        startActiveOperationExtraTimeOrFinish(par("stopOperationExtraTime"));
}

/**
     * @brief Refreshes the display information of the probabilistic routing application
     *
     */
void RlUdpApp::refreshDisplay() const
{
    ApplicationBase::refreshDisplay();

    char buf[100];
    sprintf(buf, "rcvd: %d pks\nsent: %d pks", numReceived, numSent);
    getDisplayString().setTagArg("t", 0, buf);
}

/**
     * @brief Processes the packet after splitting it based on different simulation modes
     *
     * @param pk Packet information
     */
void RlUdpApp::processPacket(Packet *pk)
{
    emit(packetReceivedSignal, pk);
    routingTable->countPktDelay(pk, (simTime() - pk->getCreationTime()).dbl());
    routingTable->stepOverJudge(pk->par("step").longValue(), simTime().dbl());
    numReceived++;
    delete pk;
}

/**
     * @brief Controls the start operation of the probabilistic routing application, sends a start message when the current time is less than the set stop time
     *
     * @param operation Application operation content
     */
void RlUdpApp::handleStartOperation(LifecycleOperation *operation)
{
    simtime_t start = std::max(startTime, simTime());
    if ((stopTime < SIMTIME_ZERO) || (start < stopTime)
        || (start == stopTime && startTime == stopTime)) {
        selfMsg->setKind(START);
        scheduleAt(start, selfMsg);
    }
}

/**
     * @brief Controls the stop operation of the probabilistic routing application. When this function is called, a stop message is sent, and the Socket is closed
     *
     * @param operation Application operation content
     */
void RlUdpApp::handleStopOperation(LifecycleOperation *operation)
{
    cancelEvent(selfMsg);
    cancelEvent(overtimeSelfMsg);
    socket.close();
    delayActiveOperationFinish(par("stopOperationTimeout"));
}

/**
     * @brief Controls the crash operation of the probabilistic routing application. When this function is called, a stop message is sent, and the Socket is destroyed
     *
     * @param operation
     */
void RlUdpApp::handleCrashOperation(LifecycleOperation *operation)
{
    cancelEvent(selfMsg);
    cancelEvent(overtimeSelfMsg);
    socket.destroy();
}

} // namespace inet