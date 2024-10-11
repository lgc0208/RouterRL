/*
 * @Author       : LIN Guocheng
 * @Date         : 2024-09-20 04:05:27
 * @LastEditors  : LIN Guocheng
 * @LastEditTime : 2024-10-09 01:13:31
 * @FilePath     : /root/RouterRL/modules/ipv4/RlPathRoutingTable.cc
 * @Description  : Path routing table in RouterRL
 */
#include "RlPathRoutingTable.h"
#include <iostream>

RlPathRoutingTable *RlPathRoutingTable::pathRoutingTable = NULL;

RlPathRoutingTable::RlPathRoutingTable()
{
}

RlPathRoutingTable::~RlPathRoutingTable()
{
    if (zmq_socket) {
        zmq_socket->close();
        delete zmq_socket;
        zmq_socket = nullptr;
        zmq_context->close();
        delete zmq_context;
    }
    if (pathRoutingTable)
        delete pathRoutingTable;
}

/**
 * @brief Used to get the unique static instance, throws an exception if the instance is not initialized
 *
 * @return RlPathRoutingTable* Probability routing table
 */
RlPathRoutingTable *RlPathRoutingTable::getInstance()
{
    return pathRoutingTable;
}

/**
 * @brief Initialize the probability routing table
 *
 * @param num               Number of nodes in the network topology
 * @param file              Initial routing probability
 * @param port              ZMQ communication port
 * @param overTime_v        Timeout value
 * @param totalStep_v       Total number of simulation steps
 * @param simMode_v         Simulation mode
 * @return RlPathRoutingTable* Initialized probability routing table
 */
RlPathRoutingTable *RlPathRoutingTable::initTable(int num, const char *initTopo_v,
                                                  const char *initRoutingTable_v, int port,
                                                  double overTime_v, int totalStep_v, int simMode_v)
{
    if (!pathRoutingTable) {
        pathRoutingTable = new RlPathRoutingTable();
        pathRoutingTable->setVals(port, num, initTopo_v, initRoutingTable_v, overTime_v,
                                  totalStep_v, simMode_v);
        pathRoutingTable->initiate();
    }
    return pathRoutingTable;
}

/**
 * @brief Initializes the instance, establishes ZMQ communication with the Python side based on TCP, and allocates memory for statistics variables
 *
 */
void RlPathRoutingTable::initiate()
{

    RlBasicRoutingTable::initiate();
    initTopoTable(initTopo, topo);
    initPathsTable(initRoutingTable);
    initCnts();
}

void RlPathRoutingTable::setVals(int port, int num, const char *initTopo_v,
                                 const char *initRoutingTable_v, double overTime_v, int totalStep_v,
                                 int returnMode_v)
{
    zmqPort = port;
    nodeNum = num;
    initTopo = initTopo_v;
    initRoutingTable = initRoutingTable_v;
    overTime = overTime_v;
    totalStep = totalStep_v;
    returnMode = returnMode_v;
}

void RlPathRoutingTable::initTopoTable(string initTopo, int **topo)
{
    istringstream iss(initTopo);
    string token;
    static int edgeCount, row, col = 0;
    while (getline(iss, token, ',')) {
        int isConnection = std::stoi(token); // Convert string to integer
        if (isConnection) {
            topo[row][col] = edgeCount;
            edgeCount++;
        } else {
            topo[row][col] = -1;
        }
        col++;
        if (col == nodeNum) {
            row++;
            col = 0;
        }
    }
    edgeNum = edgeCount;
}

void RlPathRoutingTable::initPathsTable(string initRoutingTable)
{
    string pathItem;
    stringstream ssBuffer(initRoutingTable);
    while (getline(ssBuffer, pathItem, ';')) {
        vector<string> items;
        stringstream ssItem(pathItem);
        string item;
        while (getline(ssItem, item, ',')) {
            items.push_back(item);
        }

        paths[make_pair(atoi(items[0].c_str()), atoi(items[1].c_str()))] = items[2];
    }
}

void RlPathRoutingTable::initCnts()
{
    delayWithPath.resize(nodeNum);
    for (int i = 0; i < nodeNum; ++i) {
        delayWithPath[i].resize(nodeNum);
        for (int j = 0; j < nodeNum; ++j) {
            delayWithPath[i][j].resize(totalStep);
        }
    }

    // Initialize packetSendNum
    packetSendNum.resize(nodeNum);
    for (int i = 0; i < nodeNum; ++i) {
        packetSendNum[i].resize(nodeNum);
    }

    // Initialize each element to 0
    for (int i = 0; i < nodeNum; ++i) {
        for (int j = 0; j < nodeNum; ++j) {
            packetSendNum[i][j] = vector<int>(totalStep, 0);
        }
    }
}

pair<string, int> RlPathRoutingTable::getRoute(string path, Packet *packet)
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
    pair<int, int> pktInfo = make_pair(srcNodeId, dstNodeId);
    pair<string, int> p;

    // Direct forwarding from Host to Router
    if (thisNodeName[0] == 'H') {
        if (paths[pktInfo] != "") {
            packet->addPar("path").setStringValue(paths[pktInfo].c_str());
        } else {
            string pathItem;
            stringstream ssBuffer(initRoutingTable);
            while (getline(ssBuffer, pathItem, ';')) {
                vector<string> items;
                stringstream ssItem(pathItem);
                string item;
                while (getline(ssItem, item, ',')) {
                    items.push_back(item);
                }
                if (atoi(items[0].c_str()) == srcNodeId && atoi(items[1].c_str()) == dstNodeId) {
                    packet->addPar("path").setStringValue(items[2].c_str());
                }
            }
        }

        p.first = "R[" + to_string(thisNodeId) + "]";
        p.second = 1;
        int step = packet->par("step").longValue();
        if (packetSendNum[srcNodeId][dstNodeId].size() <= step) {
            packetSendNum[srcNodeId][dstNodeId].resize(step + 1); // Initialize to 0
        }
        packetSendNum[srcNodeId][dstNodeId][step]++;
        return p;
    } else if (thisNodeId == dstNodeId) { // Direct forwarding from Router to Host
        p.first = dstNode;
        int gid = 0;
        for (int i = 0; i < nodeNum; i++) {
            if (topo[dstNodeId][i] != -1) {
                gid++;
            }
        }
        p.second = gid + 1;
        return p;
    } else {
        int nextNodeId = getNextHop(thisNodeId, packet);
        p.first = "R[" + to_string(nextNodeId) + "]";
        p.second = getGateId(thisNodeId, nextNodeId);
        countPkct(thisNodeId, nextNodeId, packet->getBitLength());
    }

    return p;
}

/**
 * @brief Get the next hop based on the forwarding probability matrix
 *
 * @param currentNode    ID of the current node
 * @param dstNode   ID of the destination node
 * @return int      Next hop node
 */
int RlPathRoutingTable::getNextHop(int currentNode, Packet *packet)
{
    string path = packet->par("path").stringValue();
    vector<int> nodes;
    string node;
    stringstream ss(path);

    // Parse the path into a list of nodes
    while (getline(ss, node, '.')) {
        nodes.push_back(atoi(node.c_str()));
    }

    // The case where currentNode equals destination is already considered before calling this function
    int nextHop = -1;
    int flag = -1;

    // Find the position of currentNode in the path and determine the next hop node
    for (int i = 0; i < nodes.size() - 1; i++) {
        if (currentNode == nodes[i]) {
            nextHop = nodes[i + 1];
            flag = i + 1;
            break;
        }
    }

    // Construct the updated path
    string updatePath = "";
    if (flag != -1) {
        for (int i = flag; i < nodes.size(); i++) {
            if (!updatePath.empty()) {
                updatePath += ".";
            }
            updatePath += to_string(nodes[i]);
        }
    }

    packet->par("path").setStringValue(updatePath.c_str());
    return nextHop;
}

/**
 * @brief Called after all packets of the current step have been sent, communicates with the ZMQ server (Python side), transfers the current step's throughput, obtains the weights of nodes for the next step, calculates the corresponding forwarding probabilities, and clears the already collected throughput data
 *
 * @param step  Current step number
 */
void RlPathRoutingTable::updateRoutingTable(int step, double stepTime)
{
    updateNodeCount[step]++;
    if (updateNodeCount[step] == nodeNum) {
        // The last node to enter the next step update
        string stateStr;
        stateStr += "s@@" + to_string(step) + "@@";
        for (int i = 0; i < nodeNum; i++)
            for (int j = 0; j < nodeNum; j++) {
                if (i == nodeNum - 1 && j == nodeNum - 1)
                    stateStr += to_string(double(pkct[i][j]) / 1000 / 1000 / stepTime);
                else
                    stateStr += to_string(double(pkct[i][j]) / 1000 / 1000 / stepTime) + ",";
            }
        clearPkts();

        const char *reqData = stateStr.c_str();
        const size_t reqLen = strlen(reqData);
        zmq::message_t request{reqLen};
        memcpy(request.data(), reqData, reqLen);
        zmq_socket->send(request, zmq::send_flags::none);

        zmq::message_t reply;
        auto res = zmq_socket->recv(reply, zmq::recv_flags::none);
        char *buffer = new char[reply.size() + 1];
        memset(buffer, 0, reply.size() + 1);
        memcpy(buffer, reply.data(), reply.size());

        // Message format: src,dst,path;src,dst,path;...
        paths.clear();
        string pathItem;
        stringstream ssBuffer(buffer);
        while (getline(ssBuffer, pathItem, ';')) {
            vector<string> items;
            stringstream ssItem(pathItem);
            string item;
            while (getline(ssItem, item, ',')) {
                items.push_back(item);
            }
            paths[make_pair(atoi(items[0].c_str()), atoi(items[1].c_str()))] = items[2];
        }

        sendId = 0; // Reset packet ID for the next step
        delete buffer;
    }
}

void RlPathRoutingTable::countPktDelay(Packet *packet, double delay)
{
    string srcNode = packet->par("src").stringValue();
    string dstNode = packet->par("dst").stringValue();
    int srcNodeId = atoi(srcNode.c_str() + 2);
    int dstNodeId = atoi(dstNode.c_str() + 2);
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
        delayWithPath[srcNodeId][dstNodeId][step].push_back(delay);
    }
}

void RlPathRoutingTable::endStep(int step)
{
    string reqStr = "r@@" + to_string(step) + "@@";
    if (returnMode == 1) {
        for (int src = 0; src < nodeNum; src++) {
            for (int dst = 0; dst < nodeNum; dst++) {
                double delay = 0.0, loss_rate = 0.0;
                if (src != dst) {
                    if (!(delayWithPath[src][dst][step].empty())) {
                        for (int i = 0; i < delayWithPath[src][dst][step].size(); i++) {
                            delay += delayWithPath[src][dst][step][i];
                        }
                        delay /= delayWithPath[src][dst][step].size();
                        loss_rate =
                            1.0
                            - delayWithPath[src][dst][step].size() / packetSendNum[src][dst][step];
                    } else {
                        loss_rate = 1.0;
                    }
                }
                reqStr += to_string(delay) + "," + to_string(loss_rate) + "/";
            }
        }
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
    cout << reqStr << endl;
    const char *reqData = reqStr.c_str();
    const size_t reqLen = strlen(reqData);
    zmq::message_t request{reqLen};
    memcpy(request.data(), reqData, reqLen);
    zmq_socket->send(request, zmq::send_flags::none);
    zmq::message_t reply;
    auto res = zmq_socket->recv(reply, zmq::recv_flags::none);

    stepFinished[step] = true;
}