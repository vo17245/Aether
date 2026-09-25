#pragma once

#include <EditorImGui/UiSystemRegistry.h>

#include <span>

namespace Aether::EditorImGui
{
enum class CorePanel
{
    DocumentTabs,
    Hierarchy,
    Inspector,
    AssetBrowser,
    Toolbar,
    Viewport,
};

// Registers independent, optional panels. Each panel reads an immutable editor
// World snapshot and sends changes through UiIntentSink.
bool RegisterCorePanels(UiSystemRegistry& systems,
    std::span<const CorePanel> panels = {});
}
