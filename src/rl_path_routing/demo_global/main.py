"""
Author       : LIN Guocheng
Date         : 2024-04-05 19:46:47
LastEditors  : LIN Guocheng
LastEditTime : 2024-09-24 07:06:47
FilePath     : /root/RouterRL/src/rl_path_routing/demo_global/main.py
Description  : The main program of running ECMP.
"""

import argparse
import json
import os
import re
import sys
import time
from argparse import Namespace
from router_rl.path_env import PathEnv

sys.path.append("src")
from rl_path_routing.demo_global.trainer import trainer


if __name__ == "__main__":
    with open(
        f"{os.path.dirname(os.path.realpath(__file__))}/hyperparameters.json",
        "r",
        encoding="utf-8",
    ) as f:
        defaults = json.load(f)

    parser = argparse.ArgumentParser()
    parser.add_argument("--routing_mode", type=str, default=defaults["routing_mode"])
    parser.add_argument("--return_mode", type=str, default=defaults["return_mode"])
    parser.add_argument("--algorithm", type=str, default=defaults["algorithm"])
    parser.add_argument("--max_ep_steps", type=int, default=defaults["max_ep_steps"])
    parser.add_argument("--topology", type=str, default=defaults["topology"])
    parser.add_argument("--flow_rate", type=float, default=defaults["flow_rate"])
    parser.add_argument("--ned_path", type=str, default=defaults["ned_path"])
    parser.add_argument("--seed", type=int, default=defaults["seed"])

    args: Namespace = parser.parse_args()

    # initialize basic parameters, no need to edit
    node_num, edge_num = 0, 0
    with open(f"{args.ned_path}/{args.topology}.ned", "r", encoding="utf-8") as file:
        ned_content = file.read()

    # find the number of routers
    router_match = re.search(r"R\[(\d+)]", ned_content)
    if not router_match:
        pass
    else:
        node_num = int(router_match.group(1))

    # find link connections between routers
    connections = re.findall(r"R\[(\d+)].*? <--> C <--> R\[(\d+)].*?;", ned_content)
    edge_num = len(connections)

    local_time = time.localtime(time.mktime(time.gmtime()) + 8 * 3600)
    date = time.strftime("%Y%m%d", local_time)
    now_time = time.strftime("%H-%M-%S", local_time)

    network_log_path = (
        f"logs/inet/{args.algorithm}-fr{args.flow_rate}-{args.topology}-{date}-{now_time}"
    )
    env = PathEnv(
        network=args.topology,
        flow_rate=args.flow_rate,
        total_step=args.max_ep_steps,
        routing_mode=args.routing_mode,
        return_mode=args.return_mode,
        seed=args.seed,
        ned_path=args.ned_path,
        log_path=network_log_path,
    )
    trainer(env, args.max_ep_steps)
    if env:
        env.close()
