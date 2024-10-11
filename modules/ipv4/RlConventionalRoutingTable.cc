/*
 * @Author       : LIN Guocheng
 * @Date         : 2024-06-12 11:37:51
 * @LastEditors  : LIN Guocheng
 * @LastEditTime : 2024-10-09 01:12:16
 * @FilePath     : /root/RouterRL/modules/ipv4/RlConventionalRoutingTable.cc
 * @Description  : Conventional routing table in RouterRL
 */
#include "RlConventionalRoutingTable.h"

RlConventionalRoutingTable *RlConventionalRoutingTable::routingTable = NULL;

RlConventionalRoutingTable::RlConventionalRoutingTable()
{
}

RlConventionalRoutingTable::~RlConventionalRoutingTable()
{
    if (zmq_socket) {
        zmq_socket->close();
        delete zmq_socket;
        zmq_socket = nullptr;
        zmq_context->close();
        delete zmq_context;
    }
    if (routingTable)
        delete routingTable;
}

/**
 * @brief Used to get the unique static instance, throws an exception if the instance is not initialized
 *
 * @return RlConventionalRoutingTable* Probability routing table
 */
RlConventionalRoutingTable *RlConventionalRoutingTable::getInstance()
{
    return routingTable;
}

void RlConventionalRoutingTable::setDeviceAddress(string deviceName, string address)
{
    deviceAddressMap[address] = deviceName;
}

string RlConventionalRoutingTable::getDeviceNameByAddress(string address)
{
    auto it = deviceAddressMap.find(address);
    if (it != deviceAddressMap.end()) {
        return it->second;
    } else {
        return "Unknown Device or Destination";
    }
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
 * @return RlConventionalRoutingTable* Initialized probability routing table
 */
RlConventionalRoutingTable *
RlConventionalRoutingTable::initTable(int nodeNum, int port, double overTime_v, int totalStep_v)
{
    if (!routingTable) {
        routingTable = new RlConventionalRoutingTable();
        routingTable->setVals(port, nodeNum, overTime_v, totalStep_v);
        routingTable->initiate();
    }
    return routingTable;
}

void RlConventionalRoutingTable::initiate()
{
    RlBasicRoutingTable::initiate();
}

void RlConventionalRoutingTable::setVals(int port, int num, double overTime_v, int totalStep_v)
{
    zmqPort = port;
    nodeNum = num;
    overTime = overTime_v;
    totalStep = totalStep_v;
}

int RlConventionalRoutingTable::getNextNode(int nodeId, int srcNode, int dstNode)
{
    return 0;
}

/**
 * @brief Called after all packets of the current step have been sent, communicates with the ZMQ server (Python side), transfers the current
 * step's throughput, obtains the weights of nodes for the next step, calculates the corresponding forwarding probabilities, and clears the 
 * already collected throughput data
 *
 * @param step  Current step number
 */
void RlConventionalRoutingTable::updateRoutingTable(int step, double stepTime)
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
        sendId = 0;
    }
}