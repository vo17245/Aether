import os
import platform
import sys
import subprocess

from Core import get_files_in_dir, is_linux, is_windows, root_path
from Log import Log

g_delete_files=[]


def create_embed(f,input_path,output_path,variable_name):
    rule_name="create_"+output_path
    rule_name=rule_name.replace("/","_")
    rule_name=rule_name.replace("\\","_")
    rule_name=rule_name.replace(".","_")
    rule_name=rule_name.replace(":","_")
    script_path=root_path("/DepotTools/src/Apps/embed.py")
    rule=f"""rule {rule_name}
    command = python3 {script_path} {input_path} {output_path} {variable_name}
"""
    output_path_ninja=output_path.replace(":","")
    build=f"""build {output_path_ninja}: {rule_name}
"""
    f.write(rule)
    f.write(build)
    
    g_delete_files.append(output_path)






        
def CreateNinja():
    Log.Info(f"AetherEditorEmbedShader CreateNinja Begin")
    path=root_path("/build.ninja")
    Log.Info(f"Create: {path}")
    with open(path,"w",encoding="utf-8") as f:
        create_embed(f,\
            "AetherEditor/shader/plane.fs",
            "AetherEditor/src/Resource/Shader/Plane_fs.cpp",
            "aether_editor_plane_fs")
        create_embed(f,\
            "AetherEditor/shader/plane.vs",
            "AetherEditor/src/Resource/Shader/Plane_vs.cpp",
            "aether_editor_plane_vs")
    Log.Info(f"AetherEditorEmbedShader CreateNinja End")
    
def Clean():
    Log.Info(f"AetherEditorEmbedShader Clear Begin")
    CreateNinja()
    for file in g_delete_files:
        if os.path.exists(file):
            Log.Info(f"Remove: {file}")
            os.remove(file)
    Log.Info(f"AetherEditorEmbedShader Clear End")

def Build():
    Log.Info(f"AetherEditorEmbedShader Build Begin")
    cwd=os.getcwd()
    os.chdir(root_path())
    subprocess.run("DepotTools/bin/x64_windows/ninja.exe")
    os.chdir(cwd)
    Log.Info(f"AetherEditorEmbedShader Build End")
    
def Run(args): 
    if len(args)!=1:
        print("Args: [CreateNinja | Clean | Build]")
        return
    action=args[0]
    
    if action=="CreateNinja":
        CreateNinja()
    elif action=="Clean":
        Clean()
    elif action=="Build":
        Build()
    else:
        print("Unknown action")
        return
    
