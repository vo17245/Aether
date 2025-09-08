## 依赖

大部分依赖通过源码引入，但是vulkan sdk需要额外处理
下载vulkan sdk: https://www.lunarg.com/vulkan-sdk/

在CMake/Local.cmake中指定vulkan sdk的位置
其他依赖被放到另一个仓库(AetherDependencies)，在CMake/Local.cmake指定另一个仓库的位置

```sh
set(VULKAN_INCLUDE_DIR "D:\\opt\\vulkan_sdk\\1.3.296.0\\Include")
set(VULKAN_LIB_DIR "D:\\opt\\vulkan_sdk\\1.3.296.0\\Lib")
set(AETHER_DEPENDENCIES_PACKAGES_DIR "D:\\dev\\AetherDependencies\\Packages")
```

**注意,vulkan sdk中include文件夹下有除了vulkan以外的库的头文件目录，比如SPIRV-Cross，需要删除掉，不然会和Aether依赖的相同的库冲突**

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

