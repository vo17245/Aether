add_executable(IO.Tests ImageTest.cpp)
target_link_libraries(IO.Tests PRIVATE IO tinyexr3::tinyexr3)
add_test(NAME IO.Tests COMMAND IO.Tests)
