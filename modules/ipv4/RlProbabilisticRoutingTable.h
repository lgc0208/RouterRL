/*
 * @Author       : LIN Guocheng
 * @Date         : 2024-05-14 17:42:23
 * @LastEditors  : LIN Guocheng
 * @LastEditTime : 2024-10-09 01:14:13
 * @FilePath     : /root/RouterRL/modules/ipv4/RlProbabilisticRoutingTable.h
 * @Description  : Probabilistic routing table in RouterRL.
 */

#ifndef RLPROBABILISTICROUTINGTABLE_H
#define RLPROBABILISTICROUTINGTABLE_H
#include "RlBasicRoutingTable.h"

/**
 * Stores the forwarding probabilities for the entire network and serves as the network's statistics module, exchanging data with the Python side through ZMQ communication.
 * Currently, there is no method for using it as a local variable, and there is only a single global static object.
 * In a multi-agent environment, it can be extended to one object per node.
 */
class RlProbabilisticRoutingTable : public RlBasicRoutingTable
{
public:
    RlProbabilisticRoutingTable();
    ~RlProbabilisticRoutingTable();
    /**
     * Used to get the unique static instance, throws an exception if the instance is not initialized.
     */
    static RlProbabilisticRoutingTable *getInstance();

    /**
     * Used to initialize the unique static instance, set parameters, and allocate memory for statistics variables.
     */
    static RlProbabilisticRoutingTable *initTable(int num, const char *file, int port,
                                                  double overTime_v, int totalStep_v,
                                                  int simMode_v);

    /**
     * Initializes the instance, establishes ZMQ communication with the Python side based on TCP, and allocates memory for statistics variables.
     */
    void initiate() override;

    void updateRoutingTable(int step, double stepTime) override;
    int getNextNode(int nodeId, int srcNode, int dstNode) override;
    void initProbTable(string initRoutingTable, float **Prob);
    void initTopoTable(string initRoutingTable, int **topo);

protected:
    float **allProb; // Stores the forwarding probabilities between all nodes, updated every step.

private:
    static RlProbabilisticRoutingTable *pTable;
};

#endif // RLPROBABILISTICROUTINGTABLE_H