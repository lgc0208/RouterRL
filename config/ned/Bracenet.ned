import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.inet.StandardHost;
import inet.node.inet.Router;
import ned.DatarateChannel;
import inet.node.ethernet.Eth1G;

network Bracenet
{
    types:
        channel C extends DatarateChannel
        {
            delay = 0.002s;
            datarate = 1Mbps;
        }
    submodules:
        H[16]: StandardHost {
            parameters:
                forwarding = true;
            gates:
                ethg[];
        }

        configurator: Ipv4NetworkConfigurator {
            parameters:
                addDefaultRoutes = false;
        }

        R[16]: Router {
            parameters:
                hasOspf = true;
        }

    connections allowunconnected:
        R[0].pppg++ <--> C <--> R[1].pppg++;
        R[0].pppg++ <--> C <--> R[3].pppg++;
        R[0].pppg++ <--> C <--> R[4].pppg++;
        R[0].pppg++ <--> C <--> R[12].pppg++;

        R[1].pppg++ <--> C <--> R[2].pppg++;
        R[1].pppg++ <--> C <--> R[5].pppg++;
        R[1].pppg++ <--> C <--> R[13].pppg++;

        R[2].pppg++ <--> C <--> R[3].pppg++;
        R[2].pppg++ <--> C <--> R[6].pppg++;
        R[2].pppg++ <--> C <--> R[14].pppg++;

        R[3].pppg++ <--> C <--> R[7].pppg++;
        R[3].pppg++ <--> C <--> R[15].pppg++;

        R[4].pppg++ <--> C <--> R[5].pppg++;
        R[4].pppg++ <--> C <--> R[7].pppg++;
        R[4].pppg++ <--> C <--> R[8].pppg++;

        R[5].pppg++ <--> C <--> R[6].pppg++;
        R[5].pppg++ <--> C <--> R[9].pppg++;

        R[6].pppg++ <--> C <--> R[7].pppg++;
        R[6].pppg++ <--> C <--> R[10].pppg++;

        R[7].pppg++ <--> C <--> R[11].pppg++;

        R[8].pppg++ <--> C <--> R[9].pppg++;
        R[8].pppg++ <--> C <--> R[11].pppg++;
        R[8].pppg++ <--> C <--> R[12].pppg++;

        R[9].pppg++ <--> C <--> R[10].pppg++;
        R[9].pppg++ <--> C <--> R[13].pppg++;

        R[10].pppg++ <--> C <--> R[11].pppg++;
        R[10].pppg++ <--> C <--> R[14].pppg++;

        R[11].pppg++ <--> C <--> R[15].pppg++;

        R[12].pppg++ <--> C <--> R[13].pppg++;
        R[12].pppg++ <--> C <--> R[15].pppg++;

        R[13].pppg++ <--> C <--> R[14].pppg++;

        R[14].pppg++ <--> C <--> R[15].pppg++;
        

        for i=0..15 {
            R[i].ethg++ <--> Eth1G <--> H[i].ethg++;
        }
}
