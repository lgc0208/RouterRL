<h1 align="center">RouterRL: The Standardized Network Simulation Framework that Supports Routing Strategies Corresponding to Significant DRL-Based Routing Approaches</h1>
<p align="center"><b>Data Science Center / School of Artificial Intelligence / Beijing University of Posts and Telecommunications</b></p>



**RouterRL** is an open-source network simulation framework based on **OMNeT++** and **INET** under knowledge-defined networking (KDN) architecture, designed to facilitate the development and testing of routing algorithms with reinforcement learning (RL). This framework provides an efficient and flexible environment for researchers and engineers to **implement and simulate main-stream RL-based routing strategies in various network scenarios**, including:

- Conventional Routing Approaches (e.g. OSPF, RIP, ECMP)
- Probabilistic Routing Approaches
- Path Selection Routing Approaches
- Multipath Routing Approaches
- Weighted Shortest Path Routing Approaches

## Table of Contents

- [Table of Contents](#table-of-contents)
- [Features](#features)
- [Directory Structure](#directory-structure)
- [Installation](#installation)
  - [Prerequisites](#prerequisites)
  - [Installation through the one-click installation script](#installation-through-the-one-click-installation-script)
  - [Installation Step-by-Step](#installation-step-by-step)
- [Usage](#usage)
  - [Running a Simulation](#running-a-simulation)
  - [Adding RL Agents](#adding-rl-agents)
- [License](#license)
- [Citation](#citation)
- [Contact](#contact)


## Features

- **Easy Installation**: Easy installation with a one-click installation script.
- **Integration with OMNeT++ and INET**: Built on the powerful OMNeT++ simulation engine and INET framework for network simulation.
- **Support for Reinforcement Learning Algorithms**: Easily integrate and test RL algorithms for routing optimization.
- **Flexible Network Configurations**: Customize topologies, traffic patterns, and routing protocols for different research needs.
- **Extensible Architecture**: Add or modify RL agents and network components with ease.

## Directory Structure

```bash
├── README.md
├── LICENSE
├── cmd 			// scripts for installation, starting and updating
	├── install.sh 		        // Script for one-click installation
	└── update.sh     		// Script for updating RouterRL
├── config 			// Configuration files
	├── omnetpp.ini 	        // Initial configuration for RouterRL
	└── ned 		        // Network topologies
├── docs 			// Documentation files
├── modules 		// Installation components for RouterRL
├── src 		    // Demos for RouterRL
└── utils 		    // Tools
	└── args_str_to_bool.py     // Used in argparse
```

## Installation

We offer both a one-piece installation script installation and a step-by-step installation. You can choose according to your needs.

### Prerequisites

- Linux OS
- Conda Environment

### Installation through the one-click installation script

1. Clone the respository:

```bash
git clone http://gitlab.local/linguocheng/RouterRL.git
```

2. Enter the code directory:

```bash
cd RouterRL
```

3. Run the one-click install script and follow the prompts:

```bash
source cmd/install.sh
```

After installation, `start.sh` will be generated in `cmd` folder. If you do not choose to automatically start the RouerRL environment in the installation script, you will need to run `source cmd/start.sh` to start the RouterRL environment each time you log in.


### Installation Step-by-Step

1. Clone the respository:

```bash
git clone http://gitlab.local/linguocheng/RouterRL.git
```

2. Enter the code directory:

```bash
cd RouterRL
```

3. Install OMNeT++

    3.1 - Make a `lib` directory and enter
    
    ```bash
    mkdir -p dir
    cd dir
    ```
    
    3.2 - Download and unzip `OMNeT++` framework

    ```bash
    wget https://github.com/omnetpp/omnetpp/releases/download/omnetpp-6.0.1/omnetpp-6.0.1-linux-x86_64.tgz
    tar zxvf omnetpp-6.0.1-linux-x86_64.tgz -C ./
    ```

    3.3 - Go to the `omnetpp-6.0.1` folder and change the following items in `configure.user` to *no*
    ```bash
    WITH_QTENV=no
    WITH_OSG=no
    WITH_OSGEARTH=no
    ```

    3.4 - Note that this version of OMNeT++ requires the Python module, so you will **need to start the corresponding conda environment before installation**. Then, Go to the `omnetpp-6.0.1` folder and enter the command
    ```bash
    . setenv
    ./configure
    ```

    In this case, if you encounter libraries that are not installed, you can install them directly using *apt*.

    3.5 - Enter the command to compile
    ```bash
    make -j$(nproc)
    ```

    3.6 - Enter the following command to test if OMNeT++ works. If it works, the installation is complete.
    ```bash
    cd samples/dyna/
    ./dyna
    ```

4. Install ZeroMQ:
```bash
apt install libzmq3-dev
```

5. Install INET:
```bash
wget https://github.com/inet-framework/inet/releases/download/v4.5.0/inet-4.5.0-src.tgz
tar zxvf inet-4.5.0-src.tgz -C ./
```
Once the decompression is complete, return to the RouterRL directory and run:

```bash
source cmd/update
```

After the installation is complete, the RouterRL environment needs to be started each time you re-login with the following command. Assuming the current directory is RouterRL, you need to run:

```bash
conda activate ENVIRONMENT_NAME
cd lib/omnetpp-6.0.1
. setenv
cd ../../
cd lib/inet4.5
. setenv
cd ../../
```

## Usage

### Running a Simulation

Once the installation is complete, try running our demo to test if RouterRL is taking effect. If you are using the one-click install script `install.sh`, the detection is already done automatically at the end of the installation. You can also try to get a better handle on RouterRL with the demos we provide.The path to the demo program includes:

```bash
src
├── convention 			
	├── ospf                //  A demo for OSPF
	└── ecmp                //  A demo for ECMP
├── rl_multipath_routing
	├── demo_distributed    //  A demo for distributed multipath routing
	└── demo_global         //  A demo for global multipath routing
├── rl_path_routing
	├── demo_distributed    //  A demo for distributed path routing
	└── demo_global         //  A demo for global path routing
└── rl_probabilistic_routing
	├── demo_distributed    //  A demo for distributed probabilistic routing
	└── demo_global         //  A demo for global probabilistic routing
```

We strongly recommend running the emulation in the home directory of the repository, for example:

```bash
cd RouterRL
python src/convention/ospf/main.py
```

For more detailed information on configuration, please see the [documentation](./docs/How%20to%20configure%20RouterRL.md) in `docs` directory.

### Adding RL Agents

Our demo provides a complete interface for RouterRL interaction. Users only need to provide state information and reward information to RL agents according to the demo, and convert the actions of RL agents into strings conforming to the RouterRL communication format to complete the addition of RL agents.

## License

This project is licensed under the GNU General Public License v3.0 - see the [LICENSE](LICENSE) file for details.

## Citation

## Contact

Guocheng Lin (linguocheng@bupt.edu.cn), Beijing University of Posts and Telecommunications.
