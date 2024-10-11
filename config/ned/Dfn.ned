import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.inet.StandardHost;
import inet.node.inet.Router;
import ned.DatarateChannel;
import inet.node.ethernet.Eth1G;

network Dfn
{
    types:
        channel C extends DatarateChannel
        {
            delay = 0.002s;
            datarate = 1Mbps;
        }
    submodules:
        H[58]: StandardHost {
            parameters:
                forwarding = true;
            gates:
                ethg[];
        }

        configurator: Ipv4NetworkConfigurator {
            parameters:
                addDefaultRoutes = false;
        }

        R[58]: Router {
            parameters:
                hasOspf = true;
        }
        

        connections allowunconnected:

            R[0].pppg++ <--> C <--> R[1].pppg++;
            R[0].pppg++ <--> C <--> R[3].pppg++;

            R[1].pppg++ <--> C <--> R[6].pppg++;
            R[1].pppg++ <--> C <--> R[15].pppg++;
            R[1].pppg++ <--> C <--> R[53].pppg++;

            R[2].pppg++ <--> C <--> R[49].pppg++;
            R[2].pppg++ <--> C <--> R[56].pppg++;

            R[3].pppg++ <--> C <--> R[52].pppg++;
            R[3].pppg++ <--> C <--> R[53].pppg++;

            R[4].pppg++ <--> C <--> R[5].pppg++;
            R[4].pppg++ <--> C <--> R[51].pppg++;

            R[5].pppg++ <--> C <--> R[10].pppg++;

            R[6].pppg++ <--> C <--> R[7].pppg++;

            R[7].pppg++ <--> C <--> R[53].pppg++;

            R[8].pppg++ <--> C <--> R[51].pppg++;

            R[9].pppg++ <--> C <--> R[51].pppg++;

            R[10].pppg++ <--> C <--> R[11].pppg++;
            R[10].pppg++ <--> C <--> R[36].pppg++;
            R[10].pppg++ <--> C <--> R[51].pppg++;

            R[11].pppg++ <--> C <--> R[43].pppg++;

            R[12].pppg++ <--> C <--> R[52].pppg++;

            R[13].pppg++ <--> C <--> R[52].pppg++;

            R[14].pppg++ <--> C <--> R[24].pppg++;
            R[14].pppg++ <--> C <--> R[27].pppg++;
            R[14].pppg++ <--> C <--> R[50].pppg++;


            R[16].pppg++ <--> C <--> R[23].pppg++;
            R[16].pppg++ <--> C <--> R[50].pppg++;

            R[17].pppg++ <--> C <--> R[25].pppg++;
            R[17].pppg++ <--> C <--> R[31].pppg++;
            R[17].pppg++ <--> C <--> R[50].pppg++;

            R[18].pppg++ <--> C <--> R[19].pppg++;
            R[18].pppg++ <--> C <--> R[38].pppg++;

            R[19].pppg++ <--> C <--> R[20].pppg++;
            R[19].pppg++ <--> C <--> R[51].pppg++;

            R[20].pppg++ <--> C <--> R[46].pppg++;

            R[21].pppg++ <--> C <--> R[22].pppg++;
            R[21].pppg++ <--> C <--> R[23].pppg++;

            R[22].pppg++ <--> C <--> R[51].pppg++;

            R[23].pppg++ <--> C <--> R[24].pppg++;


            R[25].pppg++ <--> C <--> R[27].pppg++;

            R[26].pppg++ <--> C <--> R[50].pppg++;

            R[27].pppg++ <--> C <--> R[44].pppg++;

            R[28].pppg++ <--> C <--> R[36].pppg++;
            R[28].pppg++ <--> C <--> R[51].pppg++;

            R[29].pppg++ <--> C <--> R[51].pppg++;

            R[30].pppg++ <--> C <--> R[31].pppg++;
            R[30].pppg++ <--> C <--> R[48].pppg++;


            R[32].pppg++ <--> C <--> R[33].pppg++;
            R[32].pppg++ <--> C <--> R[50].pppg++;

            R[33].pppg++ <--> C <--> R[37].pppg++;
            R[33].pppg++ <--> C <--> R[50].pppg++;

            R[34].pppg++ <--> C <--> R[35].pppg++;
            R[34].pppg++ <--> C <--> R[52].pppg++;

            R[35].pppg++ <--> C <--> R[50].pppg++;

            R[36].pppg++ <--> C <--> R[51].pppg++;

            R[37].pppg++ <--> C <--> R[52].pppg++;

            R[38].pppg++ <--> C <--> R[39].pppg++;

            R[39].pppg++ <--> C <--> R[44].pppg++;

            R[40].pppg++ <--> C <--> R[41].pppg++;
            R[40].pppg++ <--> C <--> R[43].pppg++;

            R[41].pppg++ <--> C <--> R[53].pppg++;

            R[42].pppg++ <--> C <--> R[43].pppg++;
            R[42].pppg++ <--> C <--> R[53].pppg++;

            R[43].pppg++ <--> C <--> R[51].pppg++;

            R[44].pppg++ <--> C <--> R[45].pppg++;
            R[44].pppg++ <--> C <--> R[50].pppg++;

            R[45].pppg++ <--> C <--> R[46].pppg++;

            R[46].pppg++ <--> C <--> R[51].pppg++;

            R[47].pppg++ <--> C <--> R[51].pppg++;
            R[47].pppg++ <--> C <--> R[53].pppg++;

            R[48].pppg++ <--> C <--> R[49].pppg++;
            R[48].pppg++ <--> C <--> R[52].pppg++;
            R[48].pppg++ <--> C <--> R[56].pppg++;
            R[48].pppg++ <--> C <--> R[57].pppg++;


            R[50].pppg++ <--> C <--> R[51].pppg++;
            R[50].pppg++ <--> C <--> R[52].pppg++;
            R[50].pppg++ <--> C <--> R[53].pppg++;
            R[50].pppg++ <--> C <--> R[55].pppg++;

            R[51].pppg++ <--> C <--> R[52].pppg++;
            R[51].pppg++ <--> C <--> R[53].pppg++;

            R[52].pppg++ <--> C <--> R[53].pppg++;
            R[52].pppg++ <--> C <--> R[54].pppg++;
            R[52].pppg++ <--> C <--> R[55].pppg++;
            R[52].pppg++ <--> C <--> R[56].pppg++;

            R[53].pppg++ <--> C <--> R[54].pppg++;


            R[55].pppg++ <--> C <--> R[56].pppg++;

            R[56].pppg++ <--> C <--> R[57].pppg++;


            for i=0..57 {
                R[i].ethg++ <--> Eth1G <--> H[i].ethg++;
            }
}