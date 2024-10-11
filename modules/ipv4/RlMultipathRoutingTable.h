/*
 * @Author       : LIN Guocheng
 * @Date         : 2024-05-14 17:42:23
 * @LastEditors  : LIN Guocheng
 * @LastEditTime : 2024-10-09 01:13:12
 * @FilePath     : /root/RouterRL/modules/ipv4/RlMultipathRoutingTable.h
 * @Description  : Multipath routing table in RouterRL.
 */

#ifndef RlMultipathRoutingTable_H
#define RlMultipathRoutingTable_H
#include "RlPathRoutingTable.h"

struct pair_hash_in_split {
    template <class T1, class T2>
    std::size_t operator()(const std::pair<T1, T2> &pair) const
    {
        auto hash1 = std::hash<T1>{}(pair.first);
        auto hash2 = std::hash<T2>{}(pair.second);
        return hash1 ^ hash2;
    }
};

class RlMultipathRoutingTable : public RlPathRoutingTable
{
public:
    RlMultipathRoutingTable();
    ~RlMultipathRoutingTable();
    static RlMultipathRoutingTable *getInstance();
    static RlMultipathRoutingTable *initTable(int num, const char *initTopo_v,
                                              const char *initRoutingTable_v, int port,
                                              double overTime_v, int totalStep_v, int simMode_v);
    void initiate() override;
    void updateRoutingTable(int step, double stepTime) override;
    pair<string, int> getRoute(string path, Packet *packet) override;
    void initTopoTable(string initTopo, int **topo);
    void initSplitRatioTable(string initRoutingTable);

protected:
    unordered_map<pair<int, int>, vector<pair<string, float>>, pair_hash_in_split> splitRatio;

private:
    static RlMultipathRoutingTable *multipathRoutingTable;
};

#endif // RlMultipathRoutingTable_H