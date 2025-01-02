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