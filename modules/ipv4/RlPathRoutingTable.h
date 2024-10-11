/*
 * @Author       : LIN Guocheng
 * @Date         : 2024-05-14 17:42:23
 * @LastEditors  : LIN Guocheng
 * @LastEditTime : 2024-10-09 01:13:39
 * @FilePath     : /root/RouterRL/modules/ipv4/RlPathRoutingTable.h
 * @Description  : Path routing table in RouterRL.
 */

#ifndef RLPATHROUTINGTABLE_H
#define RLPATHROUTINGTABLE_H
#include "RlBasicRoutingTable.h"

struct pair_hash_in_path {
    template <class T1, class T2>
    std::size_t operator()(const std::pair<T1, T2> &pair) const
    {
        auto hash1 = std::hash<T1>{}(pair.first);
        auto hash2 = std::hash<T2>{}(pair.second);
        return hash1 ^ hash2;
    }
};

class RlPathRoutingTable : public RlBasicRoutingTable
{
public:
    RlPathRoutingTable();
    ~RlPathRoutingTable();
    static RlPathRoutingTable *getInstance();
    static RlPathRoutingTable *initTable(int num, const char *initTopo_v,
                                         const char *initRoutingTable_v, int port,
                                         double overTime_v, int totalStep_v, int simMode_v);
    void initiate() override;
    void setVals(int port, int num, const char *initTopo_v, const char *initRoutingTable_v,
                 double overTime_v, int totalStep_v, int returnMode_v);
    void updateRoutingTable(int step, double stepTime) override;
    pair<string, int> getRoute(string path, Packet *packet) override;
    int getNextHop(int nodeId, Packet *packet);
    void initTopoTable(string initTopo, int **topo);
    void initPathsTable(string initRoutingTable);
    void initCnts();
    void countPktDelay(Packet *packet, double delay) override;
    void endStep(int step) override;

protected:
    string initTopo = "";
    unordered_map<pair<int, int>, string, pair_hash_in_path> paths;
    vector<vector<vector<vector<double>>>> delayWithPath;
    vector<vector<vector<int>>> packetSendNum;

private:
    static RlPathRoutingTable *pathRoutingTable;
};

#endif // RLPATHROUTINGTABLE_H