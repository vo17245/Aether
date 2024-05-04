import sys
import os

# root 路径
g_root_path=os.path.dirname(os.path.abspath(__file__))
g_root_path=os.path.dirname(g_root_path)
g_root_path=os.path.dirname(g_root_path)
g_root_path=g_root_path.replace("\\","/")
def root_path(path=""):
  return os.path.join(g_root_path, path)
# 添加 DepotTools/src 到 sys.path
sys.path.append(root_path("DepotTools/src"))
# 设置全局环境
from Core import set_root_path
set_root_path(g_root_path)

def main():
    if len(sys.argv)<2:
        print("Usage: python3 Core.py <app> [args]")
        return
    app=sys.argv[1]
    if app=="AetherEditorEmbedShader":
        from Apps.AetherEditorEmbedShader import Run
        Run(sys.argv[2:])
    else:
        print(f"Unknown app: {app}")
        return
main()