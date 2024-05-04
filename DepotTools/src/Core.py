import os
import platform
import sys
def is_windows():
    return platform.system() == "Windows"
def is_linux():
    return platform.system() == "Linux"
def get_files_in_dir(dir_path):
    """
    @brief
        获得文件夹下的文件名，文件名不包含路径
    """
    for root,dirs,files in os.walk(dir_path):
        break
    return files


g_root_path=""

def root_path(path=""):
    """
    @brief
        获得根目录
    """
    return g_root_path+path
def set_root_path(path):
    """
    @brief
        设置根目录
    """
    global g_root_path
    g_root_path=path