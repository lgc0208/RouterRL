/*
 * @Author       : LIN Guocheng
 * @Date         : 2024-09-20 04:05:27
 * @LastEditors  : LIN Guocheng
 * @LastEditTime : 2024-10-09 01:14:07
 * @FilePath     : /root/RouterRL/modules/ipv4/RlProbabilisticRoutingTable.cc
 * @Description  : Probabilistic routing table in RouterRL.
 */
#include "RlProbabilisticRoutingTable.h"

RlProbabilisticRoutingTable *RlProbabilisticRoutingTable::pTable = NULL;

RlProbabilisticRoutingTable::RlProbabilisticRoutingTable()
{
}

RlProbabilisticRoutingTable::~RlProbabilisticRoutingTable()
{
    if (zmq_socket) {
        zmq_socket->close();
        delete zmq_socket;
        zmq_socket = nullptr;
        zmq_context->close();
        delete zmq_context;
    }
    if (pTable)
        delete pTable;
}

/**
 * @brief Used to get the unique static instance, throws an exception if the instance is not initialized
 *
 * @return RlProbabilisticRoutingTable* Probability routing table
 */
RlProbabilisticRoutingTable *RlProbabilisticRoutingTable::getInstance()
{
    return pTable;
}

/**
 * @brief Initialize the probabilistic routing table
 *
 * @param num               Number of nodes in the network topology
 * @param file              Initial routing probability
 * @param port              ZMQ communication port
 * @param overTime_v        Timeout value
 * @param totalStep_v       Total number of simulation steps
 * @param simMode_v         Simulation mode
 * @return RlProbabilisticRoutingTable* Initialized probabilistic routing table
 */
RlProbabilisticRoutingTable *RlProbabilisticRoutingTable::initTable(int num,
                                                                    const char *initRoutingTable_v,
                                                                    int port, double overTime_v,
                                                                    int totalStep_v, int simMode_v)
{
    if (!pTable) {
        pTable = new RlProbabilisticRoutingTable();
        pTable->setVals(port, num, initRoutingTable_v, overTime_v, totalStep_v, simMode_v);
        pTable->initiate();
    }
    return pTable;
}

/**
 * @brief Initializes the instance, establishes ZMQ communication with the Python side based on TCP, and allocates memory for statistics variables
 *
 */
void RlProbabilisticRoutingTable::initiate()
{

    RlBasicRoutingTable::initiate();

    allProb = (float **)malloc(nodeNum * sizeof(float *));
    memset(allProb, 0, nodeNum * sizeof(float *));
    float *tmp_f = (float *)malloc(nodeNum * nodeNum * sizeof(float));
    memset(tmp_f, 0, nodeNum * nodeNum * sizeof(float));
    for (int i = 0; i < nodeNum; i++) {
        allProb[i] = tmp_f + nodeNum * i;
    }
    initProbTable(initRoutingTable, allProb);
    initTopoTable(initRoutingTable, topo);
}

/**
 * @brief Initialize forwarding probabilities, retrieve the initial probability forwarding table from the stored file
 *
 * @param initRoutingTable  Initial probability forwarding table
 * @param Prob      Forwarding probability matrix
 * @return int**    Forwarding probability matrix
 */
void RlProbabilisticRoutingTable::initProbTable(string initRoutingTable, float **Prob)
{
    std::istringstream iss(initRoutingTable.substr(1, initRoutingTable.size() - 2));
    std::string token;
    int row = 0;
    int col = 0;
    while (std::getline(iss, token, ',')) {
        float probability = std::stof(token); // Convert string to float
        Prob[row][col] = probability;
        col++;
        if (col == nodeNum) {
            col = 0;
            row++;
        }
    }
}

/**
 * @brief Initialize the topology information table with the initial routing table
 * 
 * @param initRoutingTable Initial probability forwarding table
 * @param topo  Initial topology table
 */
void RlProbabilisticRoutingTable::initTopoTable(string initRoutingTable, int **topo)
{
    std::istringstream iss(initRoutingTable.substr(1, initRoutingTable.size() - 2));
    std::string token;
    static int edgeCount, row, col = 0;
    while (std::getline(iss, token, ',')) {
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
    // Record the connection status. Since it's a full-duplex link, the actual number of links is divided by 2.
    edgeNum = edgeCount / 2;
}

/**
 * @brief Get the next hop based on the probability forwarding matrix
 *
 * @param nodeId    ID of the current node
 * @param dstNode   ID of the destination node
 * @return int      Next hop node
 */
int RlProbabilisticRoutingTable::getNextNode(int nodeId, int srcNode, int dstNode)
{
    if (allProb[nodeId][dstNode]) {
        return dstNode;
    } else {
        vector<int> candidateNodes;
        vector<float> candidateProbs;
        float probSum = 0.0;

        for (int i = 0; i < nodeNum; i++) {
            candidateNodes.push_back(i);
            candidateProbs.push_back(allProb[nodeId][i]);
            probSum += allProb[nodeId][i];
        }

        // Roulette wheel selection algorithm for probabilistic routing
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<> dis(0, probSum);
        float randProb = dis(gen);
        float curProb = 0.0;
        for (int i = 0; i < candidateNodes.size(); i++) {
            curProb += candidateProbs[i];
            if (randProb <= curProb)
                return candidateNodes[i];
        }
        return candidateNodes[0];
    }
}

/**
 * @brief Called after all packets of the current step have been sent, communicates with the ZMQ server (Python side), transfers the current step's throughput, obtains the weights of nodes for the next step, calculates the corresponding forwarding probabilities, and clears the already collected throughput data
 *
 * @param step  Current step number
 */
void RlProbabilisticRoutingTable::updateRoutingTable(int step, double stepTime)
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
        cout << stateStr << endl;
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
        // The received data contains (edgeNum * 2) weights, assembled into a probability table in the inet side
        char *od_prob;
        double *weights = new double[edgeNum * 2];
        for (int i = 0; i < edgeNum * 2; i++) {
            if (i == 0) {
                od_prob = strtok(buffer, ",");
            } else {
                od_prob = strtok(NULL, ",");
            }
            weights[i] = atof(od_prob);
        }

        for (int row = 0; row < nodeNum; row++) {
            double totalWeight = 0.0;
            for (int col = 0; col < nodeNum; col++) {
                if (topo[row][col] != -1) {
                    totalWeight += weights[topo[row][col]];
                }
            }
            for (int col = 0; col < nodeNum; col++) {
                if (topo[row][col] != -1) {
                    float prob = (float)(weights[topo[row][col]] / totalWeight * 100);
                    allProb[row][col] = prob;
                }
            }
        }
        delete weights;
        sendId = 0; // Reset packet ID for the next step
        delete buffer;
    }
}