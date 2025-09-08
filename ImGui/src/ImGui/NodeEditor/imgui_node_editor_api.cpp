//------------------------------------------------------------------------------
// VERSION 0.9.1
//
// LICENSE
//   This software is dual-licensed to the public domain and under the following
//   license: you are granted a perpetual, irrevocable license to copy, modify,
//   publish, and distribute this file as you see fit.
//
// CREDITS
//   Written by Michal Cichon
//------------------------------------------------------------------------------
# include "imgui_node_editor_internal.h"
# include <algorithm>


//------------------------------------------------------------------------------
static ImGui::NodeEditor::Detail::EditorContext* s_Editor = nullptr;


//------------------------------------------------------------------------------
template <typename C, typename I, typename F>
static int BuildIdList(C& container, I* list, int listSize, F&& accept)
{
    if (list != nullptr)
    {
        int count = 0;
        for (auto object : container)
        {
            if (listSize <= 0)
                break;

            if (accept(object))
            {
                list[count] = I(object->ID().AsPointer());
                ++count;
                --listSize;}
        }

        return count;
    }
    else
        return static_cast<int>(std::count_if(container.begin(), container.end(), accept));
}


//------------------------------------------------------------------------------
ImGui::NodeEditor::EditorContext* ImGui::NodeEditor::CreateEditor(const Config* config)
{
    return reinterpret_cast<ImGui::NodeEditor::EditorContext*>(new ImGui::NodeEditor::Detail::EditorContext(config));
}

void ImGui::NodeEditor::DestroyEditor(EditorContext* ctx)
{
    auto lastContext = GetCurrentEditor();

    // Set context we're about to destroy as current, to give callback valid context
    if (lastContext != ctx)
        SetCurrentEditor(ctx);

    auto editor = reinterpret_cast<ImGui::NodeEditor::Detail::EditorContext*>(ctx);

    delete editor;

    if (lastContext != ctx)
        SetCurrentEditor(lastContext);
}

const ImGui::NodeEditor::Config& ImGui::NodeEditor::GetConfig(EditorContext* ctx)
{
    if (ctx == nullptr)
        ctx = GetCurrentEditor();

    if (ctx)
    {
        auto editor = reinterpret_cast<ImGui::NodeEditor::Detail::EditorContext*>(ctx);

        return editor->GetConfig();
    }
    else
    {
        static Config s_EmptyConfig;
        return s_EmptyConfig;
    }
}

void ImGui::NodeEditor::SetCurrentEditor(EditorContext* ctx)
{
    s_Editor = reinterpret_cast<ImGui::NodeEditor::Detail::EditorContext*>(ctx);
}

ImGui::NodeEditor::EditorContext* ImGui::NodeEditor::GetCurrentEditor()
{
    return reinterpret_cast<ImGui::NodeEditor::EditorContext*>(s_Editor);
}

ImGui::NodeEditor::Style& ImGui::NodeEditor::GetStyle()
{
    return s_Editor->GetStyle();
}

const char* ImGui::NodeEditor::GetStyleColorName(StyleColor colorIndex)
{
    return s_Editor->GetStyle().GetColorName(colorIndex);
}

void ImGui::NodeEditor::PushStyleColor(StyleColor colorIndex, const ImVec4& color)
{
    s_Editor->GetStyle().PushColor(colorIndex, color);
}

void ImGui::NodeEditor::PopStyleColor(int count)
{
    s_Editor->GetStyle().PopColor(count);
}

void ImGui::NodeEditor::PushStyleVar(StyleVar varIndex, float value)
{
    s_Editor->GetStyle().PushVar(varIndex, value);
}

void ImGui::NodeEditor::PushStyleVar(StyleVar varIndex, const ImVec2& value)
{
    s_Editor->GetStyle().PushVar(varIndex, value);
}

void ImGui::NodeEditor::PushStyleVar(StyleVar varIndex, const ImVec4& value)
{
    s_Editor->GetStyle().PushVar(varIndex, value);
}

void ImGui::NodeEditor::PopStyleVar(int count)
{
    s_Editor->GetStyle().PopVar(count);
}

void ImGui::NodeEditor::Begin(const char* id, const ImVec2& size)
{
    s_Editor->Begin(id, size);
}

void ImGui::NodeEditor::End()
{
    s_Editor->End();
}

void ImGui::NodeEditor::BeginNode(NodeId id)
{
    s_Editor->GetNodeBuilder().Begin(id);
}

void ImGui::NodeEditor::BeginPin(PinId id, PinKind kind)
{
    s_Editor->GetNodeBuilder().BeginPin(id, kind);
}

void ImGui::NodeEditor::PinRect(const ImVec2& a, const ImVec2& b)
{
    s_Editor->GetNodeBuilder().PinRect(a, b);
}

void ImGui::NodeEditor::PinPivotRect(const ImVec2& a, const ImVec2& b)
{
    s_Editor->GetNodeBuilder().PinPivotRect(a, b);
}

void ImGui::NodeEditor::PinPivotSize(const ImVec2& size)
{
    s_Editor->GetNodeBuilder().PinPivotSize(size);
}

void ImGui::NodeEditor::PinPivotScale(const ImVec2& scale)
{
    s_Editor->GetNodeBuilder().PinPivotScale(scale);
}

void ImGui::NodeEditor::PinPivotAlignment(const ImVec2& alignment)
{
    s_Editor->GetNodeBuilder().PinPivotAlignment(alignment);
}

void ImGui::NodeEditor::EndPin()
{
    s_Editor->GetNodeBuilder().EndPin();
}

void ImGui::NodeEditor::Group(const ImVec2& size)
{
    s_Editor->GetNodeBuilder().Group(size);
}

void ImGui::NodeEditor::EndNode()
{
    s_Editor->GetNodeBuilder().End();
}

bool ImGui::NodeEditor::BeginGroupHint(NodeId nodeId)
{
    return s_Editor->GetHintBuilder().Begin(nodeId);
}

ImVec2 ImGui::NodeEditor::GetGroupMin()
{
    return s_Editor->GetHintBuilder().GetGroupMin();
}

ImVec2 ImGui::NodeEditor::GetGroupMax()
{
    return s_Editor->GetHintBuilder().GetGroupMax();
}

ImDrawList* ImGui::NodeEditor::GetHintForegroundDrawList()
{
    return s_Editor->GetHintBuilder().GetForegroundDrawList();
}

ImDrawList* ImGui::NodeEditor::GetHintBackgroundDrawList()
{
    return s_Editor->GetHintBuilder().GetBackgroundDrawList();
}

void ImGui::NodeEditor::EndGroupHint()
{
    s_Editor->GetHintBuilder().End();
}

ImDrawList* ImGui::NodeEditor::GetNodeBackgroundDrawList(NodeId nodeId)
{
    if (auto node = s_Editor->FindNode(nodeId))
        return s_Editor->GetNodeBuilder().GetUserBackgroundDrawList(node);
    else
        return nullptr;
}

bool ImGui::NodeEditor::Link(LinkId id, PinId startPinId, PinId endPinId, const ImVec4& color/* = ImVec4(1, 1, 1, 1)*/, float thickness/* = 1.0f*/)
{
    return s_Editor->DoLink(id, startPinId, endPinId, ImColor(color), thickness);
}

void ImGui::NodeEditor::Flow(LinkId linkId, FlowDirection direction)
{
    if (auto link = s_Editor->FindLink(linkId))
        s_Editor->Flow(link, direction);
}

bool ImGui::NodeEditor::BeginCreate(const ImVec4& color, float thickness)
{
    auto& context = s_Editor->GetItemCreator();

    if (context.Begin())
    {
        context.SetStyle(ImColor(color), thickness);
        return true;
    }
    else
        return false;
}

bool ImGui::NodeEditor::QueryNewLink(PinId* startId, PinId* endId)
{
    using Result = ImGui::NodeEditor::Detail::CreateItemAction::Result;

    auto& context = s_Editor->GetItemCreator();

    return context.QueryLink(startId, endId) == Result::True;
}

bool ImGui::NodeEditor::QueryNewLink(PinId* startId, PinId* endId, const ImVec4& color, float thickness)
{
    using Result = ImGui::NodeEditor::Detail::CreateItemAction::Result;

    auto& context = s_Editor->GetItemCreator();

    auto result = context.QueryLink(startId, endId);
    if (result != Result::Indeterminate)
        context.SetStyle(ImColor(color), thickness);

    return result == Result::True;
}

bool ImGui::NodeEditor::QueryNewNode(PinId* pinId)
{
    using Result = ImGui::NodeEditor::Detail::CreateItemAction::Result;

    auto& context = s_Editor->GetItemCreator();

    return context.QueryNode(pinId) == Result::True;
}

bool ImGui::NodeEditor::QueryNewNode(PinId* pinId, const ImVec4& color, float thickness)
{
    using Result = ImGui::NodeEditor::Detail::CreateItemAction::Result;

    auto& context = s_Editor->GetItemCreator();

    auto result = context.QueryNode(pinId);
    if (result != Result::Indeterminate)
        context.SetStyle(ImColor(color), thickness);

    return result == Result::True;
}

bool ImGui::NodeEditor::AcceptNewItem()
{
    using Result = ImGui::NodeEditor::Detail::CreateItemAction::Result;

    auto& context = s_Editor->GetItemCreator();

    return context.AcceptItem() == Result::True;
}

bool ImGui::NodeEditor::AcceptNewItem(const ImVec4& color, float thickness)
{
    using Result = ImGui::NodeEditor::Detail::CreateItemAction::Result;

    auto& context = s_Editor->GetItemCreator();

    auto result = context.AcceptItem();
    if (result != Result::Indeterminate)
        context.SetStyle(ImColor(color), thickness);

    return result == Result::True;
}

void ImGui::NodeEditor::RejectNewItem()
{
    auto& context = s_Editor->GetItemCreator();

    context.RejectItem();
}

void ImGui::NodeEditor::RejectNewItem(const ImVec4& color, float thickness)
{
    using Result = ImGui::NodeEditor::Detail::CreateItemAction::Result;

    auto& context = s_Editor->GetItemCreator();

    if (context.RejectItem() != Result::Indeterminate)
        context.SetStyle(ImColor(color), thickness);
}

void ImGui::NodeEditor::EndCreate()
{
    auto& context = s_Editor->GetItemCreator();

    context.End();
}

bool ImGui::NodeEditor::BeginDelete()
{
    auto& context = s_Editor->GetItemDeleter();

    return context.Begin();
}

bool ImGui::NodeEditor::QueryDeletedLink(LinkId* linkId, PinId* startId, PinId* endId)
{
    auto& context = s_Editor->GetItemDeleter();

    return context.QueryLink(linkId, startId, endId);
}

bool ImGui::NodeEditor::QueryDeletedNode(NodeId* nodeId)
{
    auto& context = s_Editor->GetItemDeleter();

    return context.QueryNode(nodeId);
}

bool ImGui::NodeEditor::AcceptDeletedItem(bool deleteDependencies)
{
    auto& context = s_Editor->GetItemDeleter();

    return context.AcceptItem(deleteDependencies);
}

void ImGui::NodeEditor::RejectDeletedItem()
{
    auto& context = s_Editor->GetItemDeleter();

    context.RejectItem();
}

void ImGui::NodeEditor::EndDelete()
{
    auto& context = s_Editor->GetItemDeleter();

    context.End();
}

void ImGui::NodeEditor::SetNodePosition(NodeId nodeId, const ImVec2& position)
{
    s_Editor->SetNodePosition(nodeId, position);
}

void ImGui::NodeEditor::SetGroupSize(NodeId nodeId, const ImVec2& size)
{
    s_Editor->SetGroupSize(nodeId, size);
}

ImVec2 ImGui::NodeEditor::GetNodePosition(NodeId nodeId)
{
    return s_Editor->GetNodePosition(nodeId);
}

ImVec2 ImGui::NodeEditor::GetNodeSize(NodeId nodeId)
{
    return s_Editor->GetNodeSize(nodeId);
}

void ImGui::NodeEditor::CenterNodeOnScreen(NodeId nodeId)
{
    if (auto node = s_Editor->FindNode(nodeId))
        node->CenterOnScreenInNextFrame();
}

void ImGui::NodeEditor::SetNodeZPosition(NodeId nodeId, float z)
{
    s_Editor->SetNodeZPosition(nodeId, z);
}

float ImGui::NodeEditor::GetNodeZPosition(NodeId nodeId)
{
    return s_Editor->GetNodeZPosition(nodeId);
}

void ImGui::NodeEditor::RestoreNodeState(NodeId nodeId)
{
    if (auto node = s_Editor->FindNode(nodeId))
        s_Editor->MarkNodeToRestoreState(node);
}

void ImGui::NodeEditor::Suspend()
{
    s_Editor->Suspend();
}

void ImGui::NodeEditor::Resume()
{
    s_Editor->Resume();
}

bool ImGui::NodeEditor::IsSuspended()
{
    return s_Editor->IsSuspended();
}

bool ImGui::NodeEditor::IsActive()
{
    return s_Editor->IsFocused();
}

bool ImGui::NodeEditor::HasSelectionChanged()
{
    return s_Editor->HasSelectionChanged();
}

int ImGui::NodeEditor::GetSelectedObjectCount()
{
    return (int)s_Editor->GetSelectedObjects().size();
}

int ImGui::NodeEditor::GetSelectedNodes(NodeId* nodes, int size)
{
    return BuildIdList(s_Editor->GetSelectedObjects(), nodes, size, [](auto object)
    {
        return object->AsNode() != nullptr;
    });
}

int ImGui::NodeEditor::GetSelectedLinks(LinkId* links, int size)
{
    return BuildIdList(s_Editor->GetSelectedObjects(), links, size, [](auto object)
    {
        return object->AsLink() != nullptr;
    });
}

bool ImGui::NodeEditor::IsNodeSelected(NodeId nodeId)
{
    if (auto node = s_Editor->FindNode(nodeId))
        return s_Editor->IsSelected(node);
    else
        return false;
}

bool ImGui::NodeEditor::IsLinkSelected(LinkId linkId)
{
    if (auto link = s_Editor->FindLink(linkId))
        return s_Editor->IsSelected(link);
    else
        return false;
}

void ImGui::NodeEditor::ClearSelection()
{
    s_Editor->ClearSelection();
}

void ImGui::NodeEditor::SelectNode(NodeId nodeId, bool append)
{
    if (auto node = s_Editor->FindNode(nodeId))
    {
        if (append)
            s_Editor->SelectObject(node);
        else
            s_Editor->SetSelectedObject(node);
    }
}

void ImGui::NodeEditor::SelectLink(LinkId linkId, bool append)
{
    if (auto link = s_Editor->FindLink(linkId))
    {
        if (append)
            s_Editor->SelectObject(link);
        else
            s_Editor->SetSelectedObject(link);
    }
}

void ImGui::NodeEditor::DeselectNode(NodeId nodeId)
{
    if (auto node = s_Editor->FindNode(nodeId))
        s_Editor->DeselectObject(node);
}

void ImGui::NodeEditor::DeselectLink(LinkId linkId)
{
    if (auto link = s_Editor->FindLink(linkId))
        s_Editor->DeselectObject(link);
}

bool ImGui::NodeEditor::DeleteNode(NodeId nodeId)
{
    if (auto node = s_Editor->FindNode(nodeId))
        return s_Editor->GetItemDeleter().Add(node);
    else
        return false;
}

bool ImGui::NodeEditor::DeleteLink(LinkId linkId)
{
    if (auto link = s_Editor->FindLink(linkId))
        return s_Editor->GetItemDeleter().Add(link);
    else
        return false;
}

bool ImGui::NodeEditor::HasAnyLinks(NodeId nodeId)
{
    return s_Editor->HasAnyLinks(nodeId);
}

bool ImGui::NodeEditor::HasAnyLinks(PinId pinId)
{
    return s_Editor->HasAnyLinks(pinId);
}

int ImGui::NodeEditor::BreakLinks(NodeId nodeId)
{
    return s_Editor->BreakLinks(nodeId);
}

int ImGui::NodeEditor::BreakLinks(PinId pinId)
{
    return s_Editor->BreakLinks(pinId);
}

void ImGui::NodeEditor::NavigateToContent(float duration)
{
    s_Editor->NavigateTo(s_Editor->GetContentBounds(), true, duration);
}

void ImGui::NodeEditor::NavigateToSelection(bool zoomIn, float duration)
{
    s_Editor->NavigateTo(s_Editor->GetSelectionBounds(), zoomIn, duration);
}

bool ImGui::NodeEditor::ShowNodeContextMenu(NodeId* nodeId)
{
    return s_Editor->GetContextMenu().ShowNodeContextMenu(nodeId);
}

bool ImGui::NodeEditor::ShowPinContextMenu(PinId* pinId)
{
    return s_Editor->GetContextMenu().ShowPinContextMenu(pinId);
}

bool ImGui::NodeEditor::ShowLinkContextMenu(LinkId* linkId)
{
    return s_Editor->GetContextMenu().ShowLinkContextMenu(linkId);
}

bool ImGui::NodeEditor::ShowBackgroundContextMenu()
{
    return s_Editor->GetContextMenu().ShowBackgroundContextMenu();
}

void ImGui::NodeEditor::EnableShortcuts(bool enable)
{
    s_Editor->EnableShortcuts(enable);
}

bool ImGui::NodeEditor::AreShortcutsEnabled()
{
    return s_Editor->AreShortcutsEnabled();
}

bool ImGui::NodeEditor::BeginShortcut()
{
    return s_Editor->GetShortcut().Begin();
}

bool ImGui::NodeEditor::AcceptCut()
{
    return s_Editor->GetShortcut().AcceptCut();
}

bool ImGui::NodeEditor::AcceptCopy()
{
    return s_Editor->GetShortcut().AcceptCopy();
}

bool ImGui::NodeEditor::AcceptPaste()
{
    return s_Editor->GetShortcut().AcceptPaste();
}

bool ImGui::NodeEditor::AcceptDuplicate()
{
    return s_Editor->GetShortcut().AcceptDuplicate();
}

bool ImGui::NodeEditor::AcceptCreateNode()
{
    return s_Editor->GetShortcut().AcceptCreateNode();
}

int ImGui::NodeEditor::GetActionContextSize()
{
    return static_cast<int>(s_Editor->GetShortcut().m_Context.size());
}

int ImGui::NodeEditor::GetActionContextNodes(NodeId* nodes, int size)
{
    return BuildIdList(s_Editor->GetSelectedObjects(), nodes, size, [](auto object)
    {
        return object->AsNode() != nullptr;
    });
}

int ImGui::NodeEditor::GetActionContextLinks(LinkId* links, int size)
{
    return BuildIdList(s_Editor->GetSelectedObjects(), links, size, [](auto object)
    {
        return object->AsLink() != nullptr;
    });
}

void ImGui::NodeEditor::EndShortcut()
{
    return s_Editor->GetShortcut().End();
}

float ImGui::NodeEditor::GetCurrentZoom()
{
    return s_Editor->GetView().InvScale;
}

ImGui::NodeEditor::NodeId ImGui::NodeEditor::GetHoveredNode()
{
    return s_Editor->GetHoveredNode();
}

ImGui::NodeEditor::PinId ImGui::NodeEditor::GetHoveredPin()
{
    return s_Editor->GetHoveredPin();
}

ImGui::NodeEditor::LinkId ImGui::NodeEditor::GetHoveredLink()
{
    return s_Editor->GetHoveredLink();
}

ImGui::NodeEditor::NodeId ImGui::NodeEditor::GetDoubleClickedNode()
{
    return s_Editor->GetDoubleClickedNode();
}

ImGui::NodeEditor::PinId ImGui::NodeEditor::GetDoubleClickedPin()
{
    return s_Editor->GetDoubleClickedPin();
}

ImGui::NodeEditor::LinkId ImGui::NodeEditor::GetDoubleClickedLink()
{
    return s_Editor->GetDoubleClickedLink();
}

bool ImGui::NodeEditor::IsBackgroundClicked()
{
    return s_Editor->IsBackgroundClicked();
}

bool ImGui::NodeEditor::IsBackgroundDoubleClicked()
{
    return s_Editor->IsBackgroundDoubleClicked();
}

ImGuiMouseButton ImGui::NodeEditor::GetBackgroundClickButtonIndex()
{
    return s_Editor->GetBackgroundClickButtonIndex();
}

ImGuiMouseButton ImGui::NodeEditor::GetBackgroundDoubleClickButtonIndex()
{
    return s_Editor->GetBackgroundDoubleClickButtonIndex();
}

bool ImGui::NodeEditor::GetLinkPins(LinkId linkId, PinId* startPinId, PinId* endPinId)
{
    auto link = s_Editor->FindLink(linkId);
    if (!link)
        return false;

    if (startPinId)
        *startPinId = link->m_StartPin->m_ID;
    if (endPinId)
        *endPinId = link->m_EndPin->m_ID;

    return true;
}

bool ImGui::NodeEditor::PinHadAnyLinks(PinId pinId)
{
    return s_Editor->PinHadAnyLinks(pinId);
}

ImVec2 ImGui::NodeEditor::GetScreenSize()
{
    return s_Editor->GetRect().GetSize();
}

ImVec2 ImGui::NodeEditor::ScreenToCanvas(const ImVec2& pos)
{
    return s_Editor->ToCanvas(pos);
}

ImVec2 ImGui::NodeEditor::CanvasToScreen(const ImVec2& pos)
{
    return s_Editor->ToScreen(pos);
}

int ImGui::NodeEditor::GetNodeCount()
{
    return s_Editor->CountLiveNodes();
}

int ImGui::NodeEditor::GetOrderedNodeIds(NodeId* nodes, int size)
{
    return s_Editor->GetNodeIds(nodes, size);
}
