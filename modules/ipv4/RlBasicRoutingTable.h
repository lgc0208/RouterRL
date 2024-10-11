/*
 * @Author       : LIN Guocheng
 * @Date         : 2024-06-11 10:10:23
 * @LastEditors  : LIN Guocheng
 * @LastEditTime : 2024-10-09 01:14:45
 * @FilePath     : /root/RouterRL/modules/ipv4/RlBasicRoutingTable.h
 * @Description  : Basic routing table for RL agents in RouterRL.
 */
#ifndef RLBASICROUTINGTABLE_H
#define RLBASICROUTINGTABLE_H
#include "string.h"
#include <ctime>
#include <fstream>
#include <iostream>
#include <numeric>
#include <omnetpp.h>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <zmq.hpp>
#include <random>
#include <queue>

#include "inet/networklayer/contract/INetfilter.h"

using namespace std;
using namespace omnetpp;
using namespace inet;

/**
 * Stores the forwarding probabilities for the entire network and serves as the network's statistics module, exchanging data with the Python side through ZMQ communication.
 * Currently, there is no method for using it as a local variable, and there is only a single global static object.
 * In a multi-agent environment, it can be extended to one object per node.
 */
class RlBasicRoutingTable
{
public:
    /**
     * Initializes the instance, establishes ZMQ communication with the Python side based on TCP, and allocates memory for statistics variables.
     */
    virtual void initiate();

    /**
     * Used for routing at the IP layer during packet forwarding in the pfrp protocol, determines the next hop based on the current and destination nodes, and calculates throughput.
     */
    virtual pair<string, int> getRoute(string path, Packet *packet);

    /**
     * Called after confirming the packet status of a step, calculates overall delay and packet loss rate, communicates with the ZMQ server (Python side),
     * transfers delay and packet loss rate, and clears the already collected data.
     */
    virtual void endStep(int step);
    virtual void stepOverJudge(int step, double currentTime);
    virtual void updateRoutingTable(int step, double stepTime) = 0;
    void recordPktNum(int pkNum, int stepNum);
    void countNodeEndInStep(int step, double endTime);
    virtual int getNextNode(int nodeId, int srcNode, int dstNode);
    int getGateId(int nodeId, int nextNode);
    void setVals(int port, int num, const char *initRoutingTable_v, double overTime_v,
                 int totalStep_v, int returnMode_v);
    void clearPkts();
    void countPkct(int src, int dst, int pkByte);
    virtual void
    countPktInNode(string path,
                   Packet *packet); // Only counts the nodes each packet passes through.
    // Count delay of each packet according to pktID.
    virtual void countPktDelay(Packet *packet, double delay);
    int getSendId();
    int **
        topo; // Stores the network topology, represented by -1 for no link between nodes and the link number for existing links.
    int nodeNum = 0;         // Number of nodes in the network topology.
    string initRoutingTable; // Initialization of the forwarding probability matrix.
    int **pkct;              // Records the traffic on each link during a step, measured in bits.
    int edgeNum = 0;         // Number of links in the network topology.
    vector<vector<double>> delayWithStep; // Stores delays by step number.
    int *pkNumOfStep;                     // Stores the number of packets sent in each step.
    bool *stepIsEnd;     // Sender confirms that all packets for each step have been sent.
    double *stepEndTime; // End time of each step recorded by the sender, used for st calculation.
    bool
        *stepFinished; // Confirms that each step has been passed to the RL side at the end of step.
    int *
        updateNodeCount; // Accumulator for ending steps at all nodes; the step is considered finished when it reaches nodeNum.
    int *
        stepEndRecordCount; // Accumulator for ending steps at all nodes; the step is considered finished when it reaches nodeNum.
    double overTime;        // Timeout setting.
    int totalStep = 0;      // Total number of simulation steps.

    vector<vector<unordered_set<int> *>>
        pktInNode; // Only need to count the packet IDs that pass through each node.
    vector<unordered_map<int, double> *>
        pktDelay; // Used to store the final E2E delay of each packet.

    int *stepPktNum; // Used to store the number of packets received in each step.
    int sendId = 0;  // Used to identify the packet ID, each packet in each step has a unique ID.
    int returnMode;  // Simulation mode.
    int routingMode;
    int zmqPort; // ZMQ port.
    zmq::context_t *zmq_context;
    zmq::socket_t *zmq_socket;
    RlBasicRoutingTable(); // Constructor and destructor, both private since they are not meant to be called externally.
    virtual ~RlBasicRoutingTable();
};

#endif // RLBASICROUTINGTABLE_H