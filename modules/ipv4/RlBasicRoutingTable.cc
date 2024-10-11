/*
 * @Author       : LIN Guocheng
 * @Date         : 2024-09-20 04:05:27
 * @LastEditors  : LIN Guocheng
 * @LastEditTime : 2024-09-25 09:43:59
 * @FilePath     : /root/RouterRL/modules/ipv4/RlBasicRoutingTable.cc
 * @Description  : Basic routing tabel in RouterRL
 */
#include "RlBasicRoutingTable.h"

/**
 * @brief Construct a new routing table
 *
 */
RlBasicRoutingTable::RlBasicRoutingTable()
{
}

/**
 * @brief Destroy the routing table
 *
 */
RlBasicRoutingTable::~RlBasicRoutingTable()
{
    if (zmq_socket) {
        zmq_socket->close();
        delete zmq_socket;
        zmq_socket = nullptr;
        zmq_context->close();
        delete zmq_context;
    }
}

/**
 * @brief Initialize the instance, establish ZMQ communication with the Python side based on TCP, and allocate memory for statistics variables
 *
 */
void RlBasicRoutingTable::initiate()
{
    zmq_context = new zmq::context_t(1);
    zmq_socket = new zmq::socket_t(*zmq_context, zmq::socket_type::req);
    std::string addr = "tcp://127.0.0.1:" + std::to_string(zmqPort);
    std::cout << "ZeroMQ: Connect to " << addr << std::endl;
    zmq_socket->setsockopt(ZMQ_LINGER, 0);
    zmq_socket->connect(addr);

    topo = (int **)malloc(nodeNum * sizeof(int *));
    memset(topo, 0, nodeNum);
    int *tmp = (int *)malloc(nodeNum * nodeNum * sizeof(int));
    memset(tmp, 0, nodeNum * nodeNum);
    for (int i = 0; i < nodeNum; i++) {
        topo[i] = tmp + nodeNum * i;
    }

    pkct = (int **)malloc(nodeNum * sizeof(int *));
    memset(pkct, 0, nodeNum * sizeof(int *));
    tmp = (int *)malloc(nodeNum * nodeNum * sizeof(int));
    memset(tmp, 0, nodeNum * nodeNum * sizeof(int));
    for (int i = 0; i < nodeNum; i++) {
        pkct[i] = tmp + nodeNum * i;
    }

    stepIsEnd = (bool *)malloc(totalStep * sizeof(bool));
    memset(stepIsEnd, false, totalStep * sizeof(bool));

    stepFinished = (bool *)malloc(totalStep * sizeof(bool));
    memset(stepFinished, false, totalStep * sizeof(bool));

    stepEndTime = (double *)malloc(totalStep * sizeof(double));
    memset(stepEndTime, 0, totalStep * sizeof(double));

    stepEndRecordCount = (int *)malloc(totalStep * sizeof(int));
    memset(stepEndRecordCount, 0, totalStep * sizeof(int));

    updateNodeCount = (int *)malloc(totalStep * sizeof(int));
    memset(updateNodeCount, 0, totalStep * sizeof(int));

    pkNumOfStep = (int *)malloc(totalStep * sizeof(int));
    memset(pkNumOfStep, 0, totalStep * sizeof(int));

    if (returnMode == 1) {
        for (int i = 0; i < totalStep; i++) {
            vector<unordered_set<int> *> nvec;
            for (int j = 0; j < nodeNum; j++) {
                unordered_set<int> *nuset = new unordered_set<int>;
                nvec.push_back(nuset);
            }
            pktInNode.push_back(nvec);

            unordered_map<int, double> *numap = new unordered_map<int, double>;
            pktDelay.push_back(numap);
        }

        stepPktNum = (int *)malloc(totalStep * sizeof(int));
        memset(stepPktNum, 0, totalStep * sizeof(int));
    }
}

/**
 * @brief Used for routing at the IP layer during packet forwarding in the pfrp protocol, determines the next hop based on the current and destination nodes, and calculates throughput
 *
 * @param path      Routing path
 * @param pkName    Packet name
 * @param pkBit     Packet size
 * @return pair<string, int> Next hop target node and GateID of the next hop
 */
pair<string, int> RlBasicRoutingTable::getRoute(string path, Packet *packet)
{
    char pathCpy[50] = {0};
    strncpy(pathCpy, path.c_str(), 49);
    char *locPtr = NULL;
    char *networkName = strtok_r(pathCpy, ".", &locPtr);
    char *thisNodeName = strtok_r(NULL, ".", &locPtr);
    int thisNodeId = atoi(thisNodeName + 2);

    string srcNode = packet->par("src").stringValue();
    string dstNode = packet->par("dst").stringValue();
    int srcNodeId = atoi(srcNode.c_str() + 2);
    int dstNodeId = atoi(dstNode.c_str() + 2);

    pair<string, int> p;

    // Direct forwarding from Host to Router
    if (thisNodeName[0] == 'H') {
        p.first = "R[" + to_string(thisNodeId) + "]";
        p.second = 1;
        return p;
    }

    // Direct forwarding from Router to Host
    if (thisNodeId == dstNodeId) {
        p.first = dstNode;
        int gid = 0;
        for (int i = 0; i < nodeNum; i++) {
            if (topo[dstNodeId][i] != -1) {
                gid++;
            }
        }
        // +1 because the first entry of ift is lo0, +1 because the router's interface to the host is the last one, and -1 since it starts from 0
        p.second = gid + 1;
        return p;
    }

    int nextNodeId = getNextNode(thisNodeId, srcNodeId, dstNodeId);
    p.first = "R[" + to_string(nextNodeId) + "]";
    p.second = getGateId(thisNodeId, nextNodeId);
    countPkct(thisNodeId, nextNodeId, packet->getBitLength());

    return p;
}

int RlBasicRoutingTable::getNextNode(int nodeId, int srcNode, int dstNode)
{
    return -1;
}

/**
 * @brief Record the packet size
 *
 * @param src       Source node
 * @param dst       Destination node
 * @param pkBit     Packet size, in bits
 */
void RlBasicRoutingTable::countPkct(int src, int dst, int pkBit)
{
    pkct[src][dst] += pkBit;
}

/**
 * @brief Set basic simulation parameters
 *
 * @param port              ZMQ communication port
 * @param overTime_v        Timeout value
 * @param totalStep_v       Total simulation steps
 * @param simMode_v         Simulation mode
 */
void RlBasicRoutingTable::setVals(int port, int num, const char *initRoutingTable_v,
                                  double overTime_v, int totalStep_v, int returnMode_v)
{
    zmqPort = port;
    nodeNum = num;
    initRoutingTable = initRoutingTable_v;
    overTime = overTime_v;
    totalStep = totalStep_v;
    returnMode = returnMode_v;
}

/**
 * @brief Get the GateID of the next hop node
 *
 * @param nodeId    Current node ID
 * @param nextNode  Next hop node ID
 * @return int      GateID of the next hop node
 */
int RlBasicRoutingTable::getGateId(int nodeId, int nextNode)
{

    int gateId = 0;

    // Since it's bidirectional, if node 1 can reach node 2, node 2 can definitely reach node 1
    for (int i = 0; i < nodeNum; i++) {
        if (topo[nodeId][i] != -1) {
            if (i == nextNode)
                break;
            else
                gateId++;
        }
    }
    gateId++; // +1 because the first entry of ift is lo0
    return gateId;
}

/**
 * @brief Clear packet statistics
 *
 */
void RlBasicRoutingTable::clearPkts()
{
    for (int i = 0; i < nodeNum; i++)
        for (int j = 0; j < nodeNum; j++) {
            pkct[i][j] = 0;
        }
}

/**
 * @brief Record the number of packets
 *
 * @param pkNum     Number of packets
 * @param stepNum   Current step number
 */
void RlBasicRoutingTable::recordPktNum(int pkNum, int stepNum)
{
    pkNumOfStep[stepNum] += pkNum;
}

/**
 * @brief End information statistics for the current step
 *
 * @param step      Current step number
 * @param endTime   End time
 */
void RlBasicRoutingTable::countNodeEndInStep(int step, double endTime)
{
    // This function is called when each node ends its step, an accumulator counts until all nodes finish the step
    stepEndRecordCount[step]++;
    // All nodes have finished the current step
    if (stepEndRecordCount[step] == nodeNum) {
        stepIsEnd[step] = true;
        stepEndTime[step] = endTime;
    }
}

/**
 * @brief Calculate packet information passing through each router node, including packet ID and the step number corresponding to the packet when sent
 *
 * @param path      Current path of the packet, in the format topology name.location.network protocol
 * @param pkName    Packet name
 * @return string   Updated packet name
 */
void RlBasicRoutingTable::countPktInNode(string path, Packet *packet)
{
    char pathCpy[50] = {0};
    strncpy(pathCpy, path.c_str(), 49);
    char *locPtr = NULL;
    char *networkName = strtok_r(pathCpy, ".", &locPtr);
    char *thisNodeName = strtok_r(NULL, ".", &locPtr);

    if (thisNodeName[0] == 'H') {
        return;
    } // Only statistics for routers

    int thisNodeId = atoi(thisNodeName + 2);

    int step = packet->par("step").longValue();
    int pktId = packet->par("id").longValue();
    (*pktInNode[step][thisNodeId]).insert(pktId);
}

/**
 * @brief Calculate packet delay information during multi-agent simulation, all nodes through which the packet passes receive the corresponding delay information
 *
 * @param step  Current step number
 * @param pktId Packet ID
 * @param delay Packet delay
 */
void RlBasicRoutingTable::countPktDelay(Packet *packet, double delay)
{
    int pktId = packet->par("id").longValue();
    int step = packet->par("step").longValue();
    // If delayWithStep[step] is empty, initialize delayWithStep[step] with the current delay as the first element
    if (delayWithStep.size() < (step + 1)) {
        vector<double> delayVec;
        delayVec.push_back(delay);
        delayWithStep.push_back(delayVec);
    }
    // If delayWithStep[step] already has values, insert directly
    else {
        delayWithStep[step].push_back(delay);
    }
    if (returnMode == 1) {
        (*pktDelay[step])[pktId] = delay;
    }
}

/**
 * @brief Determine if the current step is finished
 *
 * @param step          Current step number
 * @param currentTime   Current simulation time
 */
void RlBasicRoutingTable::stepOverJudge(int step, double currentTime)
{
    // Determine if all packets for this step have been sent and notify the RL side
    if (stepIsEnd[step] && (!stepFinished[step])) {
        if (delayWithStep[step].size() == pkNumOfStep[step]) {
            endStep(step);
            delayWithStep[step].clear();
        }
    }

    // Determine if all packets from previous steps have been sent and notify the RL side
    for (int formerStep = 0; formerStep <= step; formerStep++) {
        double timePast = currentTime - stepEndTime[formerStep];
        if (stepIsEnd[formerStep] && (timePast >= overTime) && (!stepFinished[formerStep])) {
            endStep(formerStep);
            delayWithStep[formerStep].clear();
        }
    }
}

/**
 * @brief End the simulation of a step
 *
 * @param step The step to be ended
 */
void RlBasicRoutingTable::endStep(int step)
{
    string reqStr = "r@@" + to_string(step) + "@@";

    if (returnMode == 1) {
        vector<int> pktPass(nodeNum, 0);
        vector<int> pktArrive(nodeNum, 0);
        vector<vector<double>> delays;
        for (int i = 0; i < nodeNum; i++) {
            vector<double> nvec;
            for (int pktId : (*pktInNode[step][i])) {
                if ((*pktDelay[step]).count(pktId)) {
                    nvec.push_back((*pktDelay[step])[pktId]);
                    pktArrive[i]++;
                }
                pktPass[i]++;
            }

            delays.push_back(nvec);
        }
        for (int i = 0; i < nodeNum; i++) {
            double sum = 0.0;
            for (int j = 0; j < delays[i].size(); j++) {
                sum += delays[i][j];
            }
            double avgDelay = (delays[i].size() == 0) ? 0.0 : (sum / delays[i].size());

            double lossRate = 1.0 - (double)(pktArrive[i]) / (double)(pktPass[i]);

            reqStr += to_string(avgDelay) + "," + to_string(lossRate) + "/";
        }
        for (int i = 0; i < nodeNum; i++) {
            pktInNode[step][i]->clear();
        }
        pktDelay[step]->clear();
    }

    double globalAvgDelay = 0.0;
    if (delayWithStep[step].size()) {
        for (int i = 0; i < delayWithStep[step].size(); i++) {
            globalAvgDelay += delayWithStep[step][i];
        }

        globalAvgDelay /= delayWithStep[step].size();
    }
    double globalLossRate = 1.0;
    if (pkNumOfStep[step] != 0)
        globalLossRate -= (double)(delayWithStep[step].size()) / (double)(pkNumOfStep[step]);
    // The final data is global information
    reqStr += to_string(globalAvgDelay) + "," + to_string(globalLossRate);

    std::cout << reqStr << std::endl;

    const char *reqData = reqStr.c_str();
    const size_t reqLen = strlen(reqData);
    zmq::message_t request{reqLen};
    memcpy(request.data(), reqData, reqLen);
    zmq_socket->send(request, zmq::send_flags::none);
    zmq::message_t reply;
    auto res = zmq_socket->recv(reply, zmq::recv_flags::none);

    stepFinished[step] = true;
}

/**
 * @brief Get the sendID
 *
 * @return int sendID+1
 */
int RlBasicRoutingTable::getSendId()
{
    return sendId++;
}