import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.inet.StandardHost;
import inet.node.inet.Router;
import ned.DatarateChannel;
import inet.node.ethernet.Eth1G;

network BtNorthAmerica
{
    types:
        channel C extends DatarateChannel
        {
            delay = 0.002s;
            datarate = 1Mbps;
        }
    submodules:
        H[36]: StandardHost {
            parameters:
                forwarding = true;
            gates:
                ethg[];
        }

        configurator: Ipv4NetworkConfigurator {
            parameters:
                addDefaultRoutes = false;
        }

        R[36]: Router {
            parameters:
                hasOspf = true;
        }

        connections:
            R[0].pppg++ <--> C <--> R[1].pppg++;
            R[0].pppg++ <--> C <--> R[2].pppg++;

            R[1].pppg++ <--> C <--> R[2].pppg++;
            R[1].pppg++ <--> C <--> R[6].pppg++;
            R[1].pppg++ <--> C <--> R[8].pppg++;
            R[1].pppg++ <--> C <--> R[22].pppg++;
            R[1].pppg++ <--> C <--> R[28].pppg++;
            R[1].pppg++ <--> C <--> R[29].pppg++;
            R[1].pppg++ <--> C <--> R[31].pppg++;

            R[2].pppg++ <--> C <--> R[3].pppg++;
            R[2].pppg++ <--> C <--> R[19].pppg++;
            R[2].pppg++ <--> C <--> R[21].pppg++;
            R[2].pppg++ <--> C <--> R[27].pppg++;
            R[2].pppg++ <--> C <--> R[28].pppg++;

            R[3].pppg++ <--> C <--> R[27].pppg++;

            R[4].pppg++ <--> C <--> R[5].pppg++;
            R[4].pppg++ <--> C <--> R[8].pppg++;
            R[4].pppg++ <--> C <--> R[15].pppg++;

            R[5].pppg++ <--> C <--> R[15].pppg++;

            R[6].pppg++ <--> C <--> R[7].pppg++;

            R[7].pppg++ <--> C <--> R[15].pppg++;
            R[7].pppg++ <--> C <--> R[17].pppg++;
            R[7].pppg++ <--> C <--> R[21].pppg++;
            R[7].pppg++ <--> C <--> R[30].pppg++;

            R[8].pppg++ <--> C <--> R[9].pppg++;
            R[8].pppg++ <--> C <--> R[21].pppg++;
            R[8].pppg++ <--> C <--> R[25].pppg++;
            R[8].pppg++ <--> C <--> R[28].pppg++;

            R[9].pppg++ <--> C <--> R[25].pppg++;
            R[9].pppg++ <--> C <--> R[28].pppg++;

            R[10].pppg++ <--> C <--> R[11].pppg++;
            R[10].pppg++ <--> C <--> R[12].pppg++;
            R[10].pppg++ <--> C <--> R[13].pppg++;
            R[10].pppg++ <--> C <--> R[14].pppg++;
            R[10].pppg++ <--> C <--> R[17].pppg++;
            R[10].pppg++ <--> C <--> R[25].pppg++;
            R[10].pppg++ <--> C <--> R[27].pppg++;

            R[11].pppg++ <--> C <--> R[12].pppg++;
            R[11].pppg++ <--> C <--> R[13].pppg++;
            R[11].pppg++ <--> C <--> R[16].pppg++;
            R[11].pppg++ <--> C <--> R[17].pppg++;

            R[13].pppg++ <--> C <--> R[16].pppg++;
            R[13].pppg++ <--> C <--> R[22].pppg++;
            R[13].pppg++ <--> C <--> R[23].pppg++;
            R[13].pppg++ <--> C <--> R[24].pppg++;
            R[13].pppg++ <--> C <--> R[25].pppg++;
            R[13].pppg++ <--> C <--> R[27].pppg++;
            R[13].pppg++ <--> C <--> R[28].pppg++;

            R[14].pppg++ <--> C <--> R[16].pppg++;

            R[15].pppg++ <--> C <--> R[28].pppg++;
            R[15].pppg++ <--> C <--> R[29].pppg++;

            R[16].pppg++ <--> C <--> R[17].pppg++;
            R[16].pppg++ <--> C <--> R[23].pppg++;

            R[17].pppg++ <--> C <--> R[22].pppg++;
            R[17].pppg++ <--> C <--> R[27].pppg++;
            R[17].pppg++ <--> C <--> R[28].pppg++;

            R[18].pppg++ <--> C <--> R[20].pppg++;
            R[18].pppg++ <--> C <--> R[25].pppg++;
            R[18].pppg++ <--> C <--> R[27].pppg++;

            R[19].pppg++ <--> C <--> R[20].pppg++;

            R[20].pppg++ <--> C <--> R[21].pppg++;

            R[22].pppg++ <--> C <--> R[23].pppg++;
            R[22].pppg++ <--> C <--> R[27].pppg++;
            R[22].pppg++ <--> C <--> R[33].pppg++;

            R[23].pppg++ <--> C <--> R[24].pppg++;
            R[23].pppg++ <--> C <--> R[25].pppg++;

            R[24].pppg++ <--> C <--> R[25].pppg++;

            R[25].pppg++ <--> C <--> R[28].pppg++;

            R[26].pppg++ <--> C <--> R[27].pppg++;
            R[26].pppg++ <--> C <--> R[35].pppg++;

            R[27].pppg++ <--> C <--> R[33].pppg++;
            R[27].pppg++ <--> C <--> R[35].pppg++;

            R[30].pppg++ <--> C <--> R[31].pppg++;

            R[32].pppg++ <--> C <--> R[33].pppg++;
            R[32].pppg++ <--> C <--> R[34].pppg++;

            R[34].pppg++ <--> C <--> R[35].pppg++;


            for i=0..35 {
                R[i].ethg++ <--> Eth1G <--> H[i].ethg++;
            }
}