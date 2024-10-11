## Overview

The configuration files in RouterRL include the `.ned` file in the `config/ned` folder, the `omnetpp.ini` file, and the `hyperparameters.json` file in the demo file in the `src` folder. This document will describe the configuration requirements in each of them.

## Configuration for `omnetpp.ini`

The top four lines of settings are used to cancel the generation of redundant files and do not need to be modified:

```bash
[General]
**.statistic-recording = false        
**.scalar-recording = false
**.vector-recording = false
**.vector-record-eventnumbers = false
```

The rest of the settings and their meanings are shown in the notes:

```bash
# The number of application for each hosts
**.H*.numApps = 1

# The type of packet-sending application, no need to edit
**.app[0].typename = "RlUdpApp" 

# The size for each packet, unit in Bytes
**.app[0].messageLength = 128

# The basic settings for RouterRL, no need to edit
**.app[0].startTime = 0s
**.app[0].localPort = 1234
**.app[0].destPort = 1234

# The TTL setting for all packets.
**.app[0].timeToLive = 255

# The overtime setting for all hosts. Packets that do not reach their destination within overtime are discarded.
**.app[0].overTime = 1
**.ipv4.ip.overTime = 1
**.app[0].stepTime = 1
```

## Configuration for `.ned` files

The .ned file is configured in the same way as in OMNeT++, please refer to the [official documentation](https://doc.omnetpp.org/omnetpp/manual/#cha:neddoc) of OMNeT++.

## Configuration for `hyperparameters.json` file

The `hyperparameters.json` file is used to set up the various types of parameters that will be used in the actual simulation. Users are encouraged to put the hyperparameters of RL intelligences in this file as well and read them via argparse. The base information of the `hyperparameters.json` file is for example:

```
{
    "return_mode": "global",
    "routing_mode": "multipath",
    "algorithm": "Multipath Routing",
    "max_ep_steps": 5,
    "topology": "Gridnet",
    "flow_rate": 0.001,
    "seed": 1,
    "ned_path": "config/ned"
}
```
Next we describe each parameter in detail：

- ***return_mode***: Customizable, including "global" and "distributed". When the value is "global", RouterRL returns only the overall average delay and packet loss rate for each time step. When the value is "distributed", RouterRL returns the overall average delay and packet loss rate for each time step, in addition to the average delay and packet loss rate for the corresponding forwarding behavior of each router. The average delay and packet loss rate corresponding to the forwarding behavior of each router is defined as the average delay and packet loss rate of all packets forwarded through it.
- ***routing_mode***: Customizable, including "convention", "path", "multipath" and "probabilistic".Different values correspond to different action formats, including:
  - *convention*: Make actions automatically, no need to edit.
  - *path*: RouterRL set the format of a path `path_1` as `src_1.hop_1.hop_2.hop_n.dst_1`. So the actions of *path* follow the format of 
    `src_1,dst_1,path_1;src_2,dst_2,path_2;src_n,dst_n,path_n`
  - *multipath*: Follow the format of
    `src_1,dst_1,path_11,ratio_11;src_1,dst_1,path_12,ratio_12;src_2,dst_2,path_21,ratio_21;src_n,dst_n,path_n,ratio_n1`
  - *probabilistic*: Follow the format of `p_1,p_2,...,p_n`, where n is 2 times the number of links. Each of these values is mapped one by one from left to right top-down to the positions in the network adjacency matrix that are not 0 (i.e., where a link exists).
- ***algorithm***: Customizable. Related to the naming of the log file for the simulation.
- ***max_ep_steps***: Customizable. Indicates how many time steps are run at a time.
- ***topology***： Customizable. Indicates the topology that needs to be simulated, and there needs to be a corresponding `.ned` file in the `config/ned` folder.
- ***flow_rate***: Customizable. Indicates the flow rate for each flow sent by packet-sending applications. Unit in *Mbps*.
- ***seed***: Customizable. Indicates random seed for RouterRL to simulate.
- ***ned_path***: No need to edit. Indicates the path of `.ned` files.