add_executable(ImGuiPacket.Tests main.cpp)
target_link_libraries(ImGuiPacket.Tests PRIVATE ImGui)
add_test(NAME ImGuiPacket.Tests COMMAND ImGuiPacket.Tests)
