import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.inet.StandardHost;
import inet.node.inet.Router;
import ned.DatarateChannel;
import inet.node.ethernet.Eth1G;

network Bbnplanet
{
    types:
        channel C extends DatarateChannel
        {
            delay = 0.002s;
            datarate = 1Mbps;
        }
    submodules:
        H[27]: StandardHost {
            parameters:
                forwarding = true;
            gates:
                ethg[];
        }

        configurator: Ipv4NetworkConfigurator {
            parameters:
                addDefaultRoutes = false;
        }

        R[27]: Router {
            parameters:
                hasOspf = true;
        }

        connections:
            R[0].pppg++ <--> C <--> R[1].pppg++;

            R[1].pppg++ <--> C <--> R[15].pppg++;
            R[1].pppg++ <--> C <--> R[18].pppg++;

            R[2].pppg++ <--> C <--> R[18].pppg++;

            R[3].pppg++ <--> C <--> R[18].pppg++;

            R[4].pppg++ <--> C <--> R[7].pppg++;

            R[5].pppg++ <--> C <--> R[7].pppg++;

            R[6].pppg++ <--> C <--> R[7].pppg++;

            R[7].pppg++ <--> C <--> R[8].pppg++;
            R[7].pppg++ <--> C <--> R[11].pppg++;

            R[8].pppg++ <--> C <--> R[9].pppg++;
            R[8].pppg++ <--> C <--> R[13].pppg++;
            R[8].pppg++ <--> C <--> R[17].pppg++;


            R[10].pppg++ <--> C <--> R[11].pppg++;

            R[11].pppg++ <--> C <--> R[12].pppg++;
            R[11].pppg++ <--> C <--> R[15].pppg++;



            R[14].pppg++ <--> C <--> R[15].pppg++;

            R[15].pppg++ <--> C <--> R[16].pppg++;
            R[15].pppg++ <--> C <--> R[23].pppg++;


            R[17].pppg++ <--> C <--> R[20].pppg++;

            R[18].pppg++ <--> C <--> R[24].pppg++;

            R[19].pppg++ <--> C <--> R[20].pppg++;

            R[20].pppg++ <--> C <--> R[21].pppg++;
            R[20].pppg++ <--> C <--> R[23].pppg++;


            R[22].pppg++ <--> C <--> R[23].pppg++;

            R[23].pppg++ <--> C <--> R[24].pppg++;

            R[24].pppg++ <--> C <--> R[25].pppg++;
            R[24].pppg++ <--> C <--> R[26].pppg++;


            for i=0..26 {
                R[i].ethg++ <--> Eth1G <--> H[i].ethg++;
            }
}