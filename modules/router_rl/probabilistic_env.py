from .base_env import BaseEnv
import os
import re
import sys
import subprocess
from typing import List, Tuple
from rich.console import Console

console = Console()


class ProbabilisticEnv(BaseEnv):
    """The class of our network simulator based on OMNeT++ and INET."""

    def __init__(
        self,
        network: str,
        flow_rate: float,
        total_step: int,
        routing_mode: int,
        return_mode: int,
        seed: int = 0,
        ned_path: str = "config/ned",
        log_path: str = "logs/inet.out",
    ):
        self.return_mode = return_mode
        super().__init__(network, flow_rate, total_step, routing_mode, seed, ned_path, log_path)
        self.node_num, self.routing_table = self.init_ned_info(
            os.path.join(ned_path, f"{network}.ned")
        )
        self.console = Console()

    def init_ned_info(self, ned_path: str) -> Tuple[int, List[int]]:
        """Initialize network basic information from ned files.

        Args:
            ned_path (str): Path of ned file to initialize.

        Returns:
            Tuple[int, List[int]]: [Number of routers, Initialized routing table]
        """
        with open(ned_path, "r", encoding="utf-8") as file:
            ned_content = file.read()

        router_match = re.search(r"R\[(\d+)\]", ned_content)

        if router_match:
            num_routers = int(router_match.group(1))
        else:
            num_routers = 0

        connections = re.findall(r"R\[(\d+)\].*? <--> C <--> R\[(\d+)\].*?;", ned_content)
        routing_table = [[0 for _ in range(num_routers)] for _ in range(num_routers)]

        for conn in connections:
            i, j = map(int, conn)
            routing_table[i][j] = 1
            routing_table[j][i] = 1

        for i in range(num_routers):
            connected_nodes = sum(routing_table[i])
            if connected_nodes > 0:
                probability = int(100 / connected_nodes)
                for j in range(num_routers):
                    if routing_table[i][j] == 1:
                        routing_table[i][j] = probability

        routing_table = [probability for row in routing_table for probability in row]

        return num_routers, routing_table

    def reset(self) -> None:
        """Reset simulator."""
        self.close()
        self.start_sim(self.log_path)
        console.log("===> Env resetted!")

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
                    f'--**.app[0].returnMode="{self.return_mode}"',
                    f'--**.app[0].routingMode="{self.routing_mode}"',
                    f'--**.configurator.routingMode="{self.routing_mode}"',
                    f"--**.app[0].flowRate={self.flow_rate}",
                    f'--**.app[0].initRoutingTable="{self.routing_table}"',
                    '--**.app[0].topoTable=""',
                    f"--**.app[0].nodeNum={self.node_num}",
                    f"--**.app[0].totalStep={self.total_step+100}",  # Warm-up period
                    f"--**.app[0].zmqPort={self.port}",
                ],
                stdin=None,
                stdout=out,
                stderr=sys.stderr,
            )

    def close(self) -> None:
        """Close simulator."""
        if self.process:
            self.process.terminate()
            try:
                self.process.wait(timeout=2)
            except subprocess.TimeoutExpired:
                os.system(f"kill {self.process.pid}")
                console.log(f"===> WAIT TIMEOUT: try `pkill {self.process.pid}`")
                self.process.wait()
