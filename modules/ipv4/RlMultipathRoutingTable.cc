/*
 * @Author       : LIN Guocheng
 * @Date         : 2024-09-20 04:05:27
 * @LastEditors  : LIN Guocheng
 * @LastEditTime : 2024-10-09 01:13:04
 * @FilePath     : /root/RouterRL/modules/ipv4/RlMultipathRoutingTable.cc
 * @Description  : Multipath routing table in RouterRL.
 */
#include "RlMultipathRoutingTable.h"
#include <iostream>

RlMultipathRoutingTable *RlMultipathRoutingTable::multipathRoutingTable = NULL;

RlMultipathRoutingTable::RlMultipathRoutingTable()
{
}

RlMultipathRoutingTable::~RlMultipathRoutingTable()
{
    if (zmq_socket) {
        zmq_socket->close();
        delete zmq_socket;
        zmq_socket = nullptr;
        zmq_context->close();
        delete zmq_context;
    }
    if (multipathRoutingTable)
        delete multipathRoutingTable;
}

/**
 * @brief Used to get the unique static instance, throws an exception if the instance is not initialized
 *
 * @return RlMultipathRoutingTable* Probability routing table
 */
RlMultipathRoutingTable *RlMultipathRoutingTable::getInstance()
{
    return multipathRoutingTable;
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
 * @return RlMultipathRoutingTable* Initialized probability routing table
 */
RlMultipathRoutingTable *RlMultipathRoutingTable::initTable(int num, const char *initTopo_v,
                                                            const char *initRoutingTable_v,
                                                            int port, double overTime_v,
                                                            int totalStep_v, int simMode_v)
{
    if (!multipathRoutingTable) {
        multipathRoutingTable = new RlMultipathRoutingTable();
        multipathRoutingTable->setVals(port, num, initTopo_v, initRoutingTable_v, overTime_v,
                                       totalStep_v, simMode_v);
        multipathRoutingTable->initiate();
    }
    return multipathRoutingTable;
}

/**
 * @brief Initializes the instance, establishes ZMQ communication with the Python side based on TCP, and allocates memory for statistics variables
 *
 */
void RlMultipathRoutingTable::initiate()
{

    RlBasicRoutingTable::initiate();
    RlPathRoutingTable::initTopoTable(initTopo, topo);
    initSplitRatioTable(initRoutingTable);
    RlPathRoutingTable::initCnts();
}

void RlMultipathRoutingTable::initTopoTable(string initTopo, int **topo)
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

void RlMultipathRoutingTable::initSplitRatioTable(string initRoutingTable)
{
    string splitItem;
    stringstream ssBuffer(initRoutingTable);
    while (getline(ssBuffer, splitItem, ';')) {
        vector<string> items;
        stringstream ssItem(splitItem);
        string item;
        while (getline(ssItem, item, ',')) {
            items.push_back(item);
        }
        splitRatio[make_pair(atoi(items[0].c_str()), atoi(items[1].c_str()))].push_back(
            make_pair(items[2], atof(items[3].c_str())));
    }
}

pair<string, int> RlMultipathRoutingTable::getRoute(string path, Packet *packet)
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
        // Assign split paths to the packet
        vector<pair<string, float>> pktSplitInfo = splitRatio[pktInfo];
        float ratioSum = 0.0;
        for (int i = 0; i < pktSplitInfo.size(); i++) {
            ratioSum += pktSplitInfo[i].second;
        }
        random_device rd;
        mt19937 gen(rd());
        uniform_real_distribution<> dis(0, ratioSum);
        float randProb = dis(gen);
        float curProb = 0.0;
        if (pktSplitInfo.size() == 1) {
            packet->addPar("path").setStringValue(pktSplitInfo[0].first.c_str());
        } else {
            for (int i = 0; i < pktSplitInfo.size(); i++) {
                curProb += pktSplitInfo[i].second;
                if (randProb < curProb) {
                    packet->addPar("path").setStringValue(pktSplitInfo[i].first.c_str());
                    break;
                }
            }
        }
        if (!packet->hasPar("path")) {
            string splitItem;
            stringstream ssBuffer(initRoutingTable);
            while (getline(ssBuffer, splitItem, ';')) {
                vector<string> items;
                stringstream ssItem(splitItem);
                string item;
                while (getline(ssItem, item, ',')) {
                    items.push_back(item);
                }
                if (atoi(items[0].c_str()) == srcNodeId && atoi(items[1].c_str()) == dstNodeId) {
                    packet->addPar("path").setStringValue(items[2].c_str());
                    break;
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
 * @brief Called after all packets of the current step have been sent, communicates with the ZMQ server (Python side), transfers the current step's throughput, obtains the weights of nodes for the next step, calculates the corresponding forwarding probabilities, and clears the already collected throughput data
 *
 * @param step  Current step number
 */
void RlMultipathRoutingTable::updateRoutingTable(int step, double stepTime)
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

        // Message format: src,dst,path,ratio;src,dst,path,ratio;...
        splitRatio.clear();
        string splitItem;
        stringstream ssBuffer(buffer);
        while (getline(ssBuffer, splitItem, ';')) {
            vector<string> items;
            stringstream ssItem(splitItem);
            string item;
            while (getline(ssItem, item, ',')) {
                items.push_back(item);
            }
            splitRatio[make_pair(atoi(items[0].c_str()), atoi(items[1].c_str()))].push_back(
                make_pair(items[2], atof(items[3].c_str())));
        }
        sendId = 0; // Reset packet ID for the next step
        delete buffer;
    }
}