"""
Author       : LIN Guocheng
Date         : 2024-05-08 11:34:22
LastEditors  : LIN Guocheng
LastEditTime : 2024-10-08 03:13:37
FilePath     : /root/RouterRL/modules/router_rl/base_env.py
Description  : Interface file of our KDN-based network simulator.
"""

import os
import sys
import subprocess
import re
from typing import Union, Tuple, List
import zmq


class BaseEnv:
    """Base class for network simulator interface."""

    def __init__(
        self,
        network: str,
        flow_rate: float,
        total_step: int,
        routing_mode: int,
        seed: int = 0,
        ned_path: str = "config/ned",
        log_path: str = "logs/inet.out",
    ):
        self.log_path = log_path
        if not os.path.exists(os.path.dirname(self.log_path)):
            os.makedirs(os.path.dirname(self.log_path))
        self.context = zmq.Context()
        self.socket = self.context.socket(zmq.REP)
        self.socket.setsockopt(zmq.RCVTIMEO, 30000)
        self.process = None

        self.ned_path = ned_path
        self.network = network
        self.flow_rate = flow_rate
        self.total_step = total_step
        self.port = self.socket.bind_to_random_port("tcp://127.0.0.1")

        self.node_num, self.routing_table = self.init_ned_info(
            os.path.join(ned_path, f"{network}.ned")
        )
        self.routing_mode = routing_mode
        self.seed = seed

    def init_ned_info(self, ned_path: str) -> Tuple[int, List[int]]:
        """Initialize network basic information from ned files.

        Args:
            ned_path (str): Path of ned file to initialize.

        Returns:
            tuple[int, list[int]]: [Number of routers, Initialized routing table]
        """
        with open(ned_path, "r", encoding="utf-8") as file:
            ned_content = file.read()

        # Find number of routers
        router_match = re.search(r"R\[(\d+)\]", ned_content)
        if router_match:
            num_routers = int(router_match.group(1))
        else:
            num_routers = 0

        routing_table = []

        return num_routers, routing_table

    def reset(self) -> None:
        """Reset simulator."""
        self.close()
        self.start_sim(self.log_path)

    def close(self) -> None:
        """Close simulator."""
        if self.process:
            self.process.terminate()
            try:
                self.process.wait(timeout=2)
            except subprocess.TimeoutExpired:
                os.system(f"kill {self.process.pid}")
                print(f"===> WAIT TIMEOUT: try `pkill {self.process.pid}`")
                self.process.wait()

    def start_sim(self, log_path: str) -> None:
        """Start network simulator.

        Args:
            log_path (str): Path for saving simulator logs.
        """
        with open(log_path, "w", encoding="utf-8") as out:
            self.process = subprocess.Popen(
                [
                    f"{os.getenv('__omnetpp_root_dir')}/bin/opp_run_release",
                    "-l",
                    f"{os.getenv('INET_ROOT')}/bin/../src/../src/INET",
                    "-x",
                    "inet.applications.voipstream;inet.common.selfdoc;inet.emulation;"
                    "inet.examples.emulation;inet.examples.voipstream;"
                    "inet.linklayer.configurator.gatescheduling.z3;inet.showcases.emulation;"
                    "inet.showcases.visualizer.osg;inet.transportlayer.tcp_lwip;"
                    "inet.visualizer.osg",
                    "-n",
                    f"{os.getenv('INET_ROOT')}/examples:{os.getenv('INET_ROOT')}/showcases:"
                    f"{os.getenv('INET_ROOT')}/src:{os.getenv('INET_ROOT')}/tests/validation:"
                    f"{os.getenv('INET_ROOT')}/tests/networks:"
                    f"{os.getenv('INET_ROOT')}/tutorials:",
                    f"--image-path={os.getenv('INET_ROOT')}/images",
                    "config/omnetpp.ini",
                    "--num-rngs=1",
                    f"--seed-0-mt={self.seed}",
                    f"--ned-path={self.ned_path}",
                    f"--network={self.network}",
                    '--**.app[0].returnMode="global"',
                    f'--**.app[0].routingMode="{self.routing_mode}"',
                    f'--**.configurator.routingMode="{self.routing_mode}"',
                    f"--**.app[0].flowRate={self.flow_rate}",
                    f'--**.app[0].initRoutingTable="{self.routing_table}"',
                    '--**.app[0].topoTable=""',
                    f"--**.app[0].nodeNum={self.node_num}",
                    f"--**.app[0].totalStep={self.total_step+100}",
                    f"--**.app[0].zmqPort={self.port}",
                ],
                stdin=None,
                stdout=out,
                stderr=sys.stderr,
            )

    def get_obs(self) -> Tuple[str, int, Union[List[float], str]]:
        """Get observation from network simulator.

        Returns:
            tuple[str, int, Union[list[float], str]]:
                flag for state or reward;
                step number for current observation;
                observation message body.
        """
        request = self.socket.recv()
        req = str(request).split("@@")
        s_or_r = req[0][2:]
        step = int(req[1])
        if s_or_r == "s":
            msg = [float(state) for state in req[2][:-1].split(",")]
        else:
            msg = req[2][:-1]
        return s_or_r, step, msg

    def make_action(self, action: str) -> None:
        """Send message to network simulator by socket.

        Args:
            action (str): Action string.
        """
        self.socket.send_string(action.strip("[]"))

    def reward_rcvd(self) -> None:
        """Get reward and return the received message."""
        self.socket.send_string("reward received")
