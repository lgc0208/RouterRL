from .base_env import BaseEnv
import os
import heapq
import re
import sys
import subprocess
from collections import defaultdict
from typing import List, Tuple
from rich.console import Console

console = Console()


class PathEnv(BaseEnv):
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
        self.console = Console()
        self.node_num, self.topo = self.init_ned_info(os.path.join(ned_path, f"{network}.ned"))
        self.topo_str = ",".join([",".join(map(str, row)) for row in self.topo])
        self.routing_table = self.get_shortest_paths()

    def init_ned_info(self, ned_path: str) -> Tuple[int, List[List[int]]]:
        """Initialize network basic information from ned files.

        Args:
            ned_path (str): Path of ned file to initialize.

        Returns:
            Tuple[int, List[List[int]]]: [Number of routers, Topo adjacency matrix]
        """
        with open(ned_path, "r", encoding="utf-8") as file:
            ned_content = file.read()

        # Find number of routers
        router_match = re.search(r"R\[(\d+)\]", ned_content)

        if router_match:
            num_routers = int(router_match.group(1))
        else:
            num_routers = 0

        # Parse connections between routers
        connections = re.findall(r"R\[(\d+)\].*? <--> C <--> R\[(\d+)\].*?;", ned_content)
        topo = [[0 for _ in range(num_routers)] for _ in range(num_routers)]

        # Fill in the adjacency matrix
        for conn in connections:
            i, j = map(int, conn)
            topo[i][j] = 1  # Assuming a connection exists, mark it as 1
            topo[j][i] = 1  # Since connections are bidirectional

        return num_routers, topo

    def find_all_shortest_paths(self, src: int, dst: int, max_paths=1000) -> List[List[int]]:
        """Find all shortest paths between source and destination.

        Args:
            src (int): Source node ID.
            dst (int): Destination node ID.
            max_paths (int, optional): Max number of paths to find. Defaults to 1000.

        Returns:
            List[List[int]]: All shortest paths.
        """
        # Initialize
        distances = [float("inf")] * self.node_num
        distances[src] = 0
        parents = defaultdict(list)
        heap = [(0, src)]

        while heap:
            current_distance, current_node = heapq.heappop(heap)
            if current_distance > distances[current_node]:
                continue
            for neighbor in range(self.node_num):
                if self.topo[current_node][neighbor] != 0:
                    weight = self.topo[current_node][neighbor]
                    distance = current_distance + weight
                    if distance < distances[neighbor]:
                        distances[neighbor] = distance
                        heapq.heappush(heap, (distance, neighbor))
                        parents[neighbor] = [current_node]
                    elif distance == distances[neighbor]:
                        parents[neighbor].append(current_node)

        # If destination is unreachable, return empty list
        if distances[dst] == float("inf"):
            return []

        # Backtrack to generate all shortest paths
        all_paths = []

        def backtrack(node, path):
            if len(all_paths) >= max_paths:
                return  # Stop backtracking if max paths reached
            if node == src:
                all_paths.append([src] + path[::-1])
                return
            for parent in parents[node]:
                backtrack(parent, path + [node])

        backtrack(dst, [])
        return all_paths

    def get_shortest_paths(self) -> str:
        """Get initial routing table based on shortest paths.

        Returns:
            str: Initial routing table as a string.
        """
        init_routing_table = ""
        self.max_candidate_path_num = 0
        for src in range(self.node_num):
            for dst in range(self.node_num):
                if src != dst:
                    all_paths = self.find_all_shortest_paths(src, dst)
                    self.max_candidate_path_num = max(len(all_paths), self.max_candidate_path_num)
                    if all_paths:
                        hop_str = ".".join(map(str, all_paths[0]))
                        init_routing_table += f"{src},{dst},{hop_str};"
        return init_routing_table[:-1]

    def reset(self) -> None:
        """Reset simulator."""
        self.close()
        self.start_sim(self.log_path)
        self.console.log("===> Env resetted!")

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
                    f'--**.app[0].topoTable="{self.topo_str}"',
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
                self.console.log(f"===> WAIT TIMEOUT: try `pkill {self.process.pid}`")
                self.process.wait()
