"""
Author       : LIN Guocheng
Date         : 2024-04-05 19:46:47
LastEditors  : LIN Guocheng
LastEditTime : 2024-10-08 07:07:42
FilePath     : /root/RouterRL/src/rl_probabilistic_routing/demo_distributed/trainer.py
Description  : Realization the training process of ECMP in RouterRL.
"""

import sys
from rich.console import Console

sys.path.append(".")
from modules.router_rl.probabilistic_env import ProbabilisticEnv

console = Console()


def trainer(
    env: ProbabilisticEnv,
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
            routing_table = [x for x in env.routing_table if x != 0]
            env.make_action(str(routing_table))
        else:
            if step < cold_start_steps:
                env.reward_rcvd()
                continue
            reward_msgs = msg.split("/")
            global_delay, global_loss_rate = map(float, reward_msgs[-1].split(","))
            console.rule(
                f"Step: {step_count}, Delay: {global_delay} s, Loss Rate: {global_loss_rate}"
            )
            for index in range(env.node_num):
                delay, loss_rate = map(float, reward_msgs[index].split(","))
                console.log(f"Node: {index}: \tDelay: {delay} s\t Loss Rate: {loss_rate}")
            average_delay += global_delay
            average_loss_rate += global_loss_rate
            step_count += 1
            env.reward_rcvd()

    average_delay /= step_count
    average_loss_rate /= step_count
    console.rule(
        f"Average Delay: {round(average_delay*1000,4)} ms\t Loss Rate: {round(average_loss_rate*100, 4)} %"
    )
    if env:
        env.close()
