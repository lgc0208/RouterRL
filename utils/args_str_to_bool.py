"""
Author       : LIN Guocheng
Date         : 2024-07-08 16:25:17
LastEditors  : LIN Guocheng
LastEditTime : 2024-09-25 09:33:09
FilePath     : /root/RouterRL/utils/args_str_to_bool.py
Description  : Used in argparse to handle input parameters of type bool.
"""
import argparse
from typing import Union

def str2bool(v: Union[str, bool]) -> bool:
    """Change parameters input throuth args from string into bool.

    Args:
        v (Union[str, bool]): Parameters input throuth args.

    Raises:
        argparse.ArgumentTypeError: If parameters input is not bool.

    Returns:
        bool: Results of input args parameters.
    """
    if isinstance(v, bool):
        return v
    if v.lower() in ("yes", "true", "t", "y", "1"):
        return True
    if v.lower() in ("no", "false", "f", "n", "0"):
        return False
    raise argparse.ArgumentTypeError("Boolean value expected.")