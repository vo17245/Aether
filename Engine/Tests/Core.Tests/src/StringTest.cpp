#include "Core/String.h"
#include <print>
#include "doctest/doctest.h"
using namespace Aether;
TEST_CASE("U8String chinese char count")
{
    U8String str("我能吞下玻璃而不伤身体");
    auto res = str.CharCount();
    CHECK(res == 11);
}

TEST_CASE("U8String to U32String")
{
    U8String str("我能吞下玻璃而不伤身体");
    auto res = str.ToU32String();
    CHECK(res.GetData().size() == 11);
    CHECK(res[0] == 25105);  // 我
    CHECK(res[1] == 33021);  // 能
    CHECK(res[2] == 21534);  // 吞
    CHECK(res[3] == 19979);  // 下
    CHECK(res[4] == 29627);  // 玻
    CHECK(res[5] == 29827);  // 璃
    CHECK(res[6] == 32780);  // 而
    CHECK(res[7] == 19981);  // 不
    CHECK(res[8] == 20260);  // 伤
    CHECK(res[9] == 36523);  // 身
    CHECK(res[10] == 20307); // 体
}

TEST_CASE("U8String operator +")
{
    U8String str1("我能吞下玻璃");
    U8String str2("而不伤身体");
    auto u8_res = str1 + str2;
    CHECK(u8_res.CharCount() == 11);
    auto u32_res = u8_res.ToU32String();
    CHECK(u32_res[0] == 25105);  // 我
    CHECK(u32_res[1] == 33021);  // 能
    CHECK(u32_res[2] == 21534);  // 吞
    CHECK(u32_res[3] == 19979);  // 下
    CHECK(u32_res[4] == 29627);  // 玻
    CHECK(u32_res[5] == 29827);  // 璃
    CHECK(u32_res[6] == 32780);  // 而
    CHECK(u32_res[7] == 19981);  // 不
    CHECK(u32_res[8] == 20260);  // 伤
    CHECK(u32_res[9] == 36523);  // 身
    CHECK(u32_res[10] == 20307); // 体
}
TEST_CASE("U8String operator +=")
{
    U8String str1("我能吞下玻璃");
    U8String str2("而不伤身体");
    auto u8_res = str1;
    u8_res += str2;
    CHECK(u8_res.CharCount() == 11);
    auto u32_res = u8_res.ToU32String();
    CHECK(u32_res[0] == 25105);  // 我
    CHECK(u32_res[1] == 33021);  // 能
    CHECK(u32_res[2] == 21534);  // 吞
    CHECK(u32_res[3] == 19979);  // 下
    CHECK(u32_res[4] == 29627);  // 玻
    CHECK(u32_res[5] == 29827);  // 璃
    CHECK(u32_res[6] == 32780);  // 而
    CHECK(u32_res[7] == 19981);  // 不
    CHECK(u32_res[8] == 20260);  // 伤
    CHECK(u32_res[9] == 36523);  // 身
    CHECK(u32_res[10] == 20307); // 体
}
TEST_CASE("U8String Find [1]")
{
    U8String str("我能吞下玻璃而不伤身体");
    auto res = str.Find("下玻璃而");
    CHECK(res.size() == 1);
    CHECK(res[0] == 3);
}
TEST_CASE("U8String Find [2]")
{
    U8String str("1111");
    auto res = str.Find("11");
    CHECK(res.size() == 3);
    CHECK(res[0] == 0);
    CHECK(res[1] == 1);
    CHECK(res[2] == 2);
}
TEST_CASE("U8String Split [1]")
{
    U8String str("我能吞下玻璃而不伤身体");
    auto res = str.Split("下玻璃而");
    CHECK(res.size() == 2);
    CHECK(res[0] == "我能吞");
    CHECK(res[1] == "不伤身体");
}

TEST_CASE("U8String Split [2]")
{
    U8String str("/home/user/Downloads/a.txt");
    auto res = str.Split("/");
    CHECK(res.size() == 5);
    CHECK(res[0] == "");
    CHECK(res[1] == "home");
    CHECK(res[2] == "user");
    CHECK(res[3] == "Downloads");
    CHECK(res[4] == "a.txt");    
}
TEST_CASE("U8String Split [3]")
{
    U8String str("aaaa");
    auto res = str.Split("aa");
    CHECK(res.size() == 3);
    CHECK(res[0] == "");
    CHECK(res[1] == "");
    CHECK(res[2] == "");
}











TEST_CASE("U32String chinese char count")
{
    U32String str("我能吞下玻璃而不伤身体");
    auto res = str.GetData().size();
    CHECK(res == 11);
}

TEST_CASE("U32String to U32String")
{
    U32String str("我能吞下玻璃而不伤身体");
    CHECK(str.GetData().size() == 11);
    CHECK(str[0] == 25105);  // 我
    CHECK(str[1] == 33021);  // 能
    CHECK(str[2] == 21534);  // 吞
    CHECK(str[3] == 19979);  // 下
    CHECK(str[4] == 29627);  // 玻
    CHECK(str[5] == 29827);  // 璃
    CHECK(str[6] == 32780);  // 而
    CHECK(str[7] == 19981);  // 不
    CHECK(str[8] == 20260);  // 伤
    CHECK(str[9] == 36523);  // 身
    CHECK(str[10] == 20307); // 体
}

TEST_CASE("U32String operator +")
{
    U32String str1("我能吞下玻璃");
    U32String str2("而不伤身体");
    U32String s=str1+str2;
    CHECK(s.Size() == 11);
    CHECK(s[0] == 25105);  // 我
    CHECK(s[1] == 33021);  // 能
    CHECK(s[2] == 21534);  // 吞
    CHECK(s[3] == 19979);  // 下
    CHECK(s[4] == 29627);  // 玻
    CHECK(s[5] == 29827);  // 璃
    CHECK(s[6] == 32780);  // 而
    CHECK(s[7] == 19981);  // 不
    CHECK(s[8] == 20260);  // 伤
    CHECK(s[9] == 36523);  // 身
    CHECK(s[10] == 20307); // 体
}
TEST_CASE("U32String operator +=")
{
    U32String str1("我能吞下玻璃");
    U32String str2("而不伤身体");
    auto s = str1;
    s += str2;
    CHECK(s.Size() == 11);
    CHECK(s[0] == 25105);  // 我
    CHECK(s[1] == 33021);  // 能
    CHECK(s[2] == 21534);  // 吞
    CHECK(s[3] == 19979);  // 下
    CHECK(s[4] == 29627);  // 玻
    CHECK(s[5] == 29827);  // 璃
    CHECK(s[6] == 32780);  // 而
    CHECK(s[7] == 19981);  // 不
    CHECK(s[8] == 20260);  // 伤
    CHECK(s[9] == 36523);  // 身
    CHECK(s[10] == 20307); // 体
}
TEST_CASE("U32String Find [1]")
{
    U32String str("我能吞下玻璃而不伤身体");
    auto res = str.Find("下玻璃而");
    CHECK(res.size() == 1);
    CHECK(res[0] == 3);
}
TEST_CASE("U32String Find [2]")
{
    U32String str("1111");
    auto res = str.Find("11");
    CHECK(res.size() == 3);
    CHECK(res[0] == 0);
    CHECK(res[1] == 1);
    CHECK(res[2] == 2);
}
TEST_CASE("U32String Split [1]")
{
    U32String str("我能吞下玻璃而不伤身体");
    auto res = str.Split("下玻璃而");
    CHECK(res.size() == 2);
    CHECK(res[0] == "我能吞");
    CHECK(res[1] == "不伤身体");
}

TEST_CASE("U32String Split [2]")
{
    U32String str("/home/user/Downloads/a.txt");
    auto res = str.Split("/");
    CHECK(res.size() == 5);
    CHECK(res[0] == "");
    CHECK(res[1] == "home");
    CHECK(res[2] == "user");
    CHECK(res[3] == "Downloads");
    CHECK(res[4] == "a.txt");    
}
TEST_CASE("U32String Split [3]")
{
    U32String str("aaaa");
    auto res = str.Split("aa");
    CHECK(res.size() == 3);
    CHECK(res[0] == "");
    CHECK(res[1] == "");
    CHECK(res[2] == "");
}