/*
 * @Author       : LIN Guocheng
 * @Date         : 2024-05-14 17:42:23
 * @LastEditors  : LIN Guocheng
 * @LastEditTime : 2024-10-09 01:12:21
 * @FilePath     : /root/RouterRL/modules/ipv4/RlConventionalRoutingTable.h
 * @Description  : Conventional routing table in RouterRL
 */

#ifndef RlConventionalRoutingTable_H
#define RlConventionalRoutingTable_H
#include "RlBasicRoutingTable.h"

/**
 * Stores the forwarding probabilities for the entire network and serves as the network's statistics module, exchanging data with the Python side through ZMQ communication.
 * Currently, there is no method for using it as a local variable, and there is only a single global static object.
 * In a multi-agent environment, it can be extended to one object per node.
 */
class RlConventionalRoutingTable : public RlBasicRoutingTable
{
public:
    RlConventionalRoutingTable();
    ~RlConventionalRoutingTable();
    /**
     * Used to get the unique static instance, throws an exception if the instance is not initialized.
     */
    static RlConventionalRoutingTable *getInstance();

    /**
     * Used to initialize the unique static instance, set parameters, and allocate memory for statistics variables.
     */
    static RlConventionalRoutingTable *initTable(int nodeNum, int port, double overTime_v,
                                                 int totalStep_v);
    void initiate() override;

    //   /**
    //    * Called after all packets of a step have been sent, communicates with the ZMQ server (Python side), transfers the current step's throughput,
    //    * obtains the weights of nodes for the next step, calculates the corresponding forwarding probabilities, and clears the already collected throughput data.
    //    */
    void updateRoutingTable(int step, double stepTime) override;
    int getNextNode(int nodeId, int srcNode, int dstNode) override;
    void setVals(int port, int nodeNum, double overTime_v, int totalStep_v);
    void setDeviceAddress(string deviceName, string address);
    string getDeviceNameByAddress(string address);

protected:
    unordered_map<string, string> deviceAddressMap;

private:
    static RlConventionalRoutingTable *routingTable;
};

#endif // RlConventionalRoutingTable_H