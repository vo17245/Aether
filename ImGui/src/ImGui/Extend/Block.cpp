#include "Block.h"

#include "AABB.h"
namespace ImGuiEx
{
static AABB g_Aabb;
static bool g_InBlock = false;
void BeginBlock()
{
    IM_ASSERT(!g_InBlock && "Nested blocks are not supported");
    g_InBlock = true;
    g_Aabb = AABB();
}
AABB EndBlock()
{
    IM_ASSERT(g_InBlock && "Mismatched EndBlock()");
    g_InBlock = false;
    return g_Aabb;
}
void BlockPushLast()
{
    IM_ASSERT(g_InBlock && "BlockPushLast() must be called between BeginBlock() and EndBlock()");
    g_Aabb.Add(ImGui::GetItemRectMax());
    g_Aabb.Add(ImGui::GetItemRectMin());
}

}