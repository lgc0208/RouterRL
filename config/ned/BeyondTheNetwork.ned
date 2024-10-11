import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.inet.StandardHost;
import inet.node.inet.Router;
import ned.DatarateChannel;
import inet.node.ethernet.Eth1G;

network BeyondTheNetwork
{
    types:
        channel C extends DatarateChannel
        {
            delay = 0.002s;
            datarate = 1Mbps;
        }
    submodules:
        H[53]: StandardHost {
            parameters:
                forwarding = true;
            gates:
                ethg[];
        }

        configurator: Ipv4NetworkConfigurator {
            parameters:
                addDefaultRoutes = false;
        }

        R[53]: Router {
            parameters:
                hasOspf = true;
        }


        connections allowunconnected:
            // 格式：
            // 一个个节点写链接，从小到大
            // 写每一个节点的链路的时候右边的节点也要从小到大 0-1 0-2...
            // 写完router之间的再写router和host的
            R[0].pppg++ <--> C <--> R[3].pppg++;

            R[1].pppg++ <--> C <--> R[47].pppg++;

            R[2].pppg++ <--> C <--> R[3].pppg++;
            R[2].pppg++ <--> C <--> R[25].pppg++;
            R[2].pppg++ <--> C <--> R[44].pppg++;
            R[2].pppg++ <--> C <--> R[51].pppg++;

            R[3].pppg++ <--> C <--> R[13].pppg++;
            R[3].pppg++ <--> C <--> R[21].pppg++;
            R[3].pppg++ <--> C <--> R[22].pppg++;
            R[3].pppg++ <--> C <--> R[31].pppg++;
            R[3].pppg++ <--> C <--> R[33].pppg++;
            R[3].pppg++ <--> C <--> R[47].pppg++;

            R[4].pppg++ <--> C <--> R[5].pppg++;
            R[4].pppg++ <--> C <--> R[7].pppg++;

            R[5].pppg++ <--> C <--> R[8].pppg++;
            R[5].pppg++ <--> C <--> R[12].pppg++;
            R[5].pppg++ <--> C <--> R[19].pppg++;
            R[5].pppg++ <--> C <--> R[20].pppg++;
            R[5].pppg++ <--> C <--> R[45].pppg++;

            R[6].pppg++ <--> C <--> R[7].pppg++;
            R[6].pppg++ <--> C <--> R[10].pppg++;
            R[6].pppg++ <--> C <--> R[14].pppg++;
            R[6].pppg++ <--> C <--> R[26].pppg++;
            R[6].pppg++ <--> C <--> R[47].pppg++;


            R[8].pppg++ <--> C <--> R[9].pppg++;

            R[9].pppg++ <--> C <--> R[11].pppg++;
            R[9].pppg++ <--> C <--> R[27].pppg++;
            R[9].pppg++ <--> C <--> R[28].pppg++;
            R[9].pppg++ <--> C <--> R[35].pppg++;
            R[9].pppg++ <--> C <--> R[37].pppg++;
            R[9].pppg++ <--> C <--> R[38].pppg++;

            R[15].pppg++ <--> C <--> R[52].pppg++;

            R[16].pppg++ <--> C <--> R[35].pppg++;

            R[17].pppg++ <--> C <--> R[49].pppg++;

            R[18].pppg++ <--> C <--> R[40].pppg++;

            R[23].pppg++ <--> C <--> R[51].pppg++;

            R[24].pppg++ <--> C <--> R[40].pppg++;

            R[25].pppg++ <--> C <--> R[43].pppg++;
            R[25].pppg++ <--> C <--> R[47].pppg++;

            R[29].pppg++ <--> C <--> R[49].pppg++;

            R[30].pppg++ <--> C <--> R[47].pppg++;


            R[32].pppg++ <--> C <--> R[35].pppg++;


            R[34].pppg++ <--> C <--> R[47].pppg++;

            R[35].pppg++ <--> C <--> R[36].pppg++;
            R[35].pppg++ <--> C <--> R[41].pppg++;
            R[35].pppg++ <--> C <--> R[42].pppg++;
            R[35].pppg++ <--> C <--> R[48].pppg++;
            R[35].pppg++ <--> C <--> R[49].pppg++;

            R[36].pppg++ <--> C <--> R[37].pppg++;
            R[36].pppg++ <--> C <--> R[42].pppg++;

            R[37].pppg++ <--> C <--> R[38].pppg++;


            R[39].pppg++ <--> C <--> R[40].pppg++;
            R[39].pppg++ <--> C <--> R[41].pppg++;

            R[40].pppg++ <--> C <--> R[49].pppg++;

            R[41].pppg++ <--> C <--> R[42].pppg++;


            R[43].pppg++ <--> C <--> R[44].pppg++;
            R[43].pppg++ <--> C <--> R[50].pppg++;
            R[43].pppg++ <--> C <--> R[52].pppg++;


            R[45].pppg++ <--> C <--> R[47].pppg++;

            R[46].pppg++ <--> C <--> R[47].pppg++;

            R[47].pppg++ <--> C <--> R[48].pppg++;


            R[49].pppg++ <--> C <--> R[50].pppg++;
            R[49].pppg++ <--> C <--> R[51].pppg++;


            R[51].pppg++ <--> C <--> R[52].pppg++;


            for i=0..52 {
                R[i].ethg++ <--> Eth1G <--> H[i].ethg++;
            }
}