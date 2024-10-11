"""
Author       : LIN Guocheng
Date         : 2024-05-08 16:34:49
LastEditors  : LIN Guocheng
LastEditTime : 2024-09-24 02:29:26
FilePath     : /root/RouterRL/src/convention/ospf/main.py
Description  : Main process of running OSPF protocol.
"""

import os
import json
import sys
import argparse
import time
from rich.console import Console

console = Console()
from router_rl.conventional_env import ConventionalEnv

sys.path.append("src")
from convention.ospf.trainer import trainer

if __name__ == "__main__":
    with open(
        f"{os.path.dirname(os.path.realpath(__file__))}/hyperparameters.json",
        "r",
        encoding="utf-8",
    ) as f:
        defaults = json.load(f)

    parser = argparse.ArgumentParser()
    parser.add_argument("--routing_mode", type=str, default=defaults["routing_mode"])
    parser.add_argument("--algorithm", type=str, default=defaults["algorithm"])
    parser.add_argument("--max_ep_steps", type=int, default=defaults["max_ep_steps"])
    parser.add_argument("--topology", type=str, default=defaults["topology"])
    parser.add_argument("--flow_rate", type=float, default=defaults["flow_rate"])
    parser.add_argument("--ned_path", type=str, default=defaults["ned_path"])
    parser.add_argument("--seed", type=int, default=defaults["seed"])
    args = parser.parse_args()

    local_time = time.localtime(time.mktime(time.gmtime()) + 8 * 3600)
    date = time.strftime("%Y%m%d", local_time)
    now_time = time.strftime("%H-%M-%S", local_time)

    network_log_path = f"logs/inet/\
{args.algorithm}-fr{args.flow_rate}-{args.topology}-{date}-{now_time}"
    console.log(f"network log path: {network_log_path}")
    env = ConventionalEnv(
        network=args.topology,
        flow_rate=args.flow_rate,
        total_step=args.max_ep_steps,
        routing_mode=args.routing_mode,
        seed=args.seed,
        ned_path=args.ned_path,
        log_path=network_log_path,
    )
    trainer(env, args.max_ep_steps)
    if env:
        env.close()
