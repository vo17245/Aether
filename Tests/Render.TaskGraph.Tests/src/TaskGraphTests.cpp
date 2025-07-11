#include "doctest/doctest.h"

TEST_CASE("Test Preprocessor")
{
    Filesystem::CreateDirectory("tmp/Render/Test");
    std::span<uint8_t> mainCppData((uint8_t*)mainCpp, sizeof(mainCpp));
    Filesystem::WriteFile("tmp/Render/Test/main.cpp", mainCppData);
    std::span<uint8_t> funcHData((uint8_t*)funcH, sizeof(funcH));
    Filesystem::WriteFile("tmp/Render/Test/func.h", funcHData);
    Shader::Preprocessor preprocessor;
    preprocessor.AddInclude("tmp/Render/Test");
    auto res=preprocessor.Preprocess(mainCpp);
    
}