## 依赖

大部分依赖通过源码引入，但是vulkan sdk需要额外处理
下载vulkan sdk: https://www.lunarg.com/vulkan-sdk/

在CMake/Local.cmake中指定vulkan sdk的位置

```sh
set(VULKAN_INCLUDE_DIR "D:\\vulkan_sdk\\Include")
set(VULKAN_LIB_DIR "D:\\vulkan_sdk\\Lib")
```

## 编译



### windows

require: visual studio 2022 or higher

```sh
cd Script
.\CreateProject_VS.bat
.\Build.bat
```

## 开发

开发环境使用vscode
需要的插件
- clangd  
- Clang-Format 
- C/C++ 


## vscode config

.vscode/launch.json
```json
{
    "version": "0.2.0",
    "configurations": [
        {
            "name": "(Windows) Launch",
            "type": "cppvsdbg",
            "request": "launch",
            "program": "${workspaceFolder}/Build/UI.Tests/Demo/Debug/UI.Tests.Demo.exe",
            "cwd": "${workspaceFolder}",
            "args": [],
            "stopAtEntry": false,
            "preLaunchTask": "build",
            "environment": [],
            "console": "externalTerminal"
        },
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
        "command": "cmake --build Build --config Debug",
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

## 单元测试

require: python3


### Windows
```
cd Script
.\RunUnitTest_VS.bat
```

