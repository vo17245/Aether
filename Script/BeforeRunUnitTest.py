# 执行这个脚本工作目录要在Scirpt目录
import os
import shutil
import subprocess


os.chdir("..")
if os.path.exists("tmp"):
    shutil.rmtree("tmp")
os.mkdir("tmp")
os.chdir("Script")
