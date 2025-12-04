import subprocess
import sys
import threading
from .log import info
def run_command(command:list[str]):
    cmd_str=" ".join(command)
    info(f"Running command: {cmd_str}")
    result=subprocess.run(command,stdout=None,stderr=None)
    if result.returncode!=0:
        raise subprocess.CalledProcessError(result.returncode,command)

    


