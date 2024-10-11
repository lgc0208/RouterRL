"""
Author       : LIN Guocheng
Date         : 2024-04-05 19:46:47
LastEditors  : LIN Guocheng
LastEditTime : 2024-09-23 08:37:52
FilePath     : /root/RouterRL/rl-path-routing/src/demo_global/trainer.py
Description  : Realization the training process of ECMP in RouterRL.
"""

import sys
from rich.console import Console

sys.path.append(".")
from modules.router_rl.path_env import PathEnv

console = Console()


def trainer(
    env: PathEnv,
    max_ep_steps: int,
    cold_start_steps: int = 20,
) -> None:
    """Start ECMP in RouterRL.

    Args:
        env (OmnetEnv): Simulator instance.
        max_ep_steps (int): Max number of steps to simulate.
        cold_start_steps (int, optional): Step number for cold starting. Defaults to 20.
    """
    env.reset()
    step_count = 0
    average_delay = 0
    average_loss_rate = 0
    while step_count < max_ep_steps:
        s_or_r, step, msg = env.get_obs()

        if s_or_r == "s":
            env.make_action(str(env.routing_table))
        else:
            if step < cold_start_steps:
                env.reward_rcvd()
                continue
            delay, loss_rate = float(msg.split(",")[0]), float(msg.split(",")[1])
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
