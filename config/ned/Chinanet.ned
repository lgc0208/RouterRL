import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.inet.StandardHost;
import inet.node.inet.Router;
import ned.DatarateChannel;
import inet.node.ethernet.Eth1G;

network Chinanet
{
    types:
        channel C extends DatarateChannel
        {
            delay = 0.002s;
            datarate = 1Mbps;
        }
    submodules:
        H[42]: StandardHost {
            parameters:
                forwarding = true;
            gates:
                ethg[];
        }

        configurator: Ipv4NetworkConfigurator {
            parameters:
                addDefaultRoutes = false;
        }

        R[42]: Router {
            parameters:
                hasOspf = true;
        }

        connections allowunconnected:
            R[0].pppg++ <--> C <--> R[3].pppg++;
            R[0].pppg++ <--> C <--> R[16].pppg++;
            R[0].pppg++ <--> C <--> R[39].pppg++;

            R[1].pppg++ <--> C <--> R[18].pppg++;
            R[1].pppg++ <--> C <--> R[39].pppg++;

            R[2].pppg++ <--> C <--> R[33].pppg++;


            R[4].pppg++ <--> C <--> R[8].pppg++;

            R[5].pppg++ <--> C <--> R[38].pppg++;

            R[6].pppg++ <--> C <--> R[18].pppg++;
            R[6].pppg++ <--> C <--> R[39].pppg++;

            R[7].pppg++ <--> C <--> R[39].pppg++;

            R[8].pppg++ <--> C <--> R[9].pppg++;
            R[8].pppg++ <--> C <--> R[11].pppg++;
            R[8].pppg++ <--> C <--> R[16].pppg++;
            R[8].pppg++ <--> C <--> R[18].pppg++;
            R[8].pppg++ <--> C <--> R[23].pppg++;
            R[8].pppg++ <--> C <--> R[24].pppg++;
            R[8].pppg++ <--> C <--> R[25].pppg++;
            R[8].pppg++ <--> C <--> R[26].pppg++;
            R[8].pppg++ <--> C <--> R[27].pppg++;
            R[8].pppg++ <--> C <--> R[28].pppg++;
            R[8].pppg++ <--> C <--> R[31].pppg++;
            R[8].pppg++ <--> C <--> R[38].pppg++;
            R[8].pppg++ <--> C <--> R[39].pppg++;

            R[9].pppg++ <--> C <--> R[27].pppg++;

            R[10].pppg++ <--> C <--> R[39].pppg++;


            R[12].pppg++ <--> C <--> R[28].pppg++;

            R[13].pppg++ <--> C <--> R[25].pppg++;

            R[14].pppg++ <--> C <--> R[16].pppg++;
            R[14].pppg++ <--> C <--> R[28].pppg++;

            R[15].pppg++ <--> C <--> R[16].pppg++;
            R[15].pppg++ <--> C <--> R[28].pppg++;

            R[16].pppg++ <--> C <--> R[27].pppg++;
            R[16].pppg++ <--> C <--> R[28].pppg++;

            R[17].pppg++ <--> C <--> R[28].pppg++;

            R[18].pppg++ <--> C <--> R[25].pppg++;
            R[18].pppg++ <--> C <--> R[27].pppg++;
            R[18].pppg++ <--> C <--> R[28].pppg++;
            R[18].pppg++ <--> C <--> R[32].pppg++;
            R[18].pppg++ <--> C <--> R[33].pppg++;
            R[18].pppg++ <--> C <--> R[39].pppg++;
            R[18].pppg++ <--> C <--> R[40].pppg++;
            R[18].pppg++ <--> C <--> R[41].pppg++;

            R[19].pppg++ <--> C <--> R[39].pppg++;

            R[20].pppg++ <--> C <--> R[23].pppg++;

            R[21].pppg++ <--> C <--> R[28].pppg++;

            R[22].pppg++ <--> C <--> R[25].pppg++;
            R[22].pppg++ <--> C <--> R[28].pppg++;

            R[23].pppg++ <--> C <--> R[28].pppg++;
            R[23].pppg++ <--> C <--> R[39].pppg++;


            R[25].pppg++ <--> C <--> R[27].pppg++;
            R[25].pppg++ <--> C <--> R[39].pppg++;


            R[27].pppg++ <--> C <--> R[30].pppg++;
            R[27].pppg++ <--> C <--> R[39].pppg++;

            R[28].pppg++ <--> C <--> R[29].pppg++;
            R[28].pppg++ <--> C <--> R[38].pppg++;
            R[28].pppg++ <--> C <--> R[39].pppg++;




            R[32].pppg++ <--> C <--> R[39].pppg++;

            R[33].pppg++ <--> C <--> R[39].pppg++;

            R[34].pppg++ <--> C <--> R[39].pppg++;

            R[35].pppg++ <--> C <--> R[39].pppg++;

            R[36].pppg++ <--> C <--> R[39].pppg++;

            R[37].pppg++ <--> C <--> R[38].pppg++;

            R[38].pppg++ <--> C <--> R[39].pppg++;

            R[39].pppg++ <--> C <--> R[40].pppg++;
            R[39].pppg++ <--> C <--> R[41].pppg++;


            for i=0..41 {
                R[i].ethg++ <--> Eth1G <--> H[i].ethg++;
            }
}