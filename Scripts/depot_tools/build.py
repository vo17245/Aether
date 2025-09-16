from core.run_command import run_command
import core.log as log
work_dir="../../"
def run_in_work_dir(cmd):
    import os
    cur_dir=os.getcwd()
    os.chdir(work_dir)
    rc=run_command(cmd)
    os.chdir(cur_dir)
    return rc
rc=run_in_work_dir(["cmake","--build","Build","--config","Debug"])
if rc==0:
    log.info("Build succeeded!")
else:
    log.error("Build failed!")