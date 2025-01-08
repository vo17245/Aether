## 依赖

大部分依赖通过源码引入，但是vulkan sdk需要额外处理
下载vulkan sdk: https://www.lunarg.com/vulkan-sdk/

在CMake/Local.cmake中指定vulkan sdk的位置

```sh
set(VULKAN_INCLUDE_DIR "D:\\vulkan_sdk\\Include")
set(VULKAN_LIB_DIR "D:\\vulkan_sdk\\Lib")
```

## 编译

工具链
- llvm 
- ninja 
- cmake

运行Script/CreateProject.bat 生成项目
运行Script/Build.bat 编译

## 开发

开发环境使用vscode
需要的插件
- clangd  
- Clang-Format 
- C/C++ Extension Pack  
- CodeLLDB 

在运行Script/CreateProject.bat 之后，拷贝Build\compile_commands.json 作为clangd配置文件

## vscode config

.vscode/launch.json
```json
{
    // Use IntelliSense to learn about possible attributes.
    // Hover to view descriptions of existing attributes.
    // For more information, visit: https://go.microsoft.com/fwlink/?linkid=830387
    "version": "0.2.0",
    "configurations": [
    {
        "type": "lldb",
        "request": "launch",
        "name": "Launch",
        //"program": "${workspaceFolder}/Build/Render.Tests/Unit/Render.Tests.Unit.exe",
        //"program": "${workspaceFolder}/Build/Core.Tests/Core.Tests.exe",
        "program": "${workspaceFolder}/Build/Filesystem.Tests/Filesystem.Tests.exe",
        //"program": "${workspaceFolder}/Build/Window.Tests/Window.Tests.exe",
        "args": [],
        "cwd": "${workspaceFolder}",
        "preLaunchTask": "build"
    }
    ]
}
```

.vscode/tasks.json
```json
{
  "version": "2.0.0",
  "tasks": [
    {
      "label": "build",
      "type": "shell",
      "command": "cmake --build Build --config Debug > Build/build.log",
      "group": {
        "kind": "build",
        "isDefault": true
      },
      "problemMatcher": [],
      //"options": {
      //"cwd": "${workspaceFolder}/Script"
      //},
    }
  ]
}
```