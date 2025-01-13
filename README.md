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

#### 选择工具链

项目使用cmake编译，cmake会自己选择工具链，如果需要指定cmake使用的工具链的话，修改Script/CreateProject.bat
example:
指定使用mingw
```
-DCMAKE_C_COMPILER="C:/mingw/bin/gcc.exe" -DCMAKE_CXX_COMPILER="C:/mingw/bin/g++.exe"

```
#### 使用llvm-clang或者visual studio

运行Script/CreateProject.bat 生成项目
运行Script/Build.bat 编译

#### 使用mingw

在CMake/Local.cmake中添加对libstdc++exp.a的链接，用于支持c++23的std::print
这个库文件在mingw安装目录下
```
link_libraries("C:\\opt\\w64devkit\\lib\\gcc\\x86_64-w64-mingw32\\14.2.0\\libstdc++exp.a")
```
然后运行Script/CreateProject.bat 生成项目
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

## 单元测试

所有单元测试的工作目录都在项目根目录
运行
> Script/RunUnitTest.bat
来运行所有单元测试
注意: 需要安装python3
