"""
Author       : LIN Guocheng
Date         : 2024-05-08 11:34:22
LastEditors  : LIN Guocheng
LastEditTime : 2024-09-24 08:33:08
FilePath     : /root/RouterRL/modules/router_rl/conventional_env.py
Description  : Interface file of our KDN-based network simulator.
"""

from .base_env import BaseEnv


class ConventionalEnv(BaseEnv):
    """The interface file between Python and OMNeT++/INET."""

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
        super().__init__(network, flow_rate, total_step, routing_mode, seed, ned_path, log_path)
