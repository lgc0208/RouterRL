"""
Author       : LIN Guocheng
Date         : 2024-05-08 16:34:49
LastEditors  : LIN Guocheng
LastEditTime : 2024-09-23 08:32:47
FilePath     : /root/RouterRL/conventional-ospf/src/ospf/trainer.py
Description  : Start OSPF in RouterRL.
"""

import sys
from rich.console import Console

console = Console()
sys.path.append(".")
from modules.router_rl.conventional_env import ConventionalEnv


def trainer(
    env: ConventionalEnv,
    max_steps: int,
    cold_start_steps: int = 20,
) -> None:
    """Start OSPF in RouterRL.

    Args:
        env (OmnetEnv): Simulator instance.
        max_steps (int): The number of steps for a episode.
        cold_start_steps (int): The number of steps for cold starting.
    """
    env.reset()
    step_count = 0
    average_delay = 0
    average_loss_rate = 0
    while step_count < max_steps:
        s_or_r, step, msg = env.get_obs()

        if s_or_r == "s":
            env.make_action("get state")
        else:
            if step < cold_start_steps:
                env.reward_rcvd()
                continue
            delay = float(msg.split(",")[0])
            loss_rate = float(msg.split(",")[1])
            console.log(f"Step: {step_count}, Delay: {delay} s, Loss Rate: {loss_rate}")
            average_delay += delay
            average_loss_rate += loss_rate
            step_count += 1
            env.reward_rcvd()
    average_delay /= step_count
    average_loss_rate /= step_count
    console.rule(
        f"Average Delay: {round(average_delay*1000,4)} ms\t Loss Rate: {round(average_loss_rate*100, 4)} %"
    )
    if env:
        env.close()
