#pragma once

#include <EditorFramework/AssetEditor.h>
#include <EditorFramework/ComponentSchema.h>
#include <ProjectAsset/Types.h>

#include <expected>
#include <string>
#include <unordered_map>

namespace Aether::EditorFramework
{
struct ViewportProviderDescriptor
{
    GameFeatures::FeatureId ownerFeature;
    std::string id;
};

class EditorFeatureRegistrar
{
public:
    using Result = std::expected<void, std::string>;
    explicit EditorFeatureRegistrar(GameFeatures::FeatureDescriptor feature) : m_Feature(std::move(feature)) {}
    Result RegisterComponentEditor(ComponentEditorDescriptor descriptor, const GameFeatures::RuntimeRegistry& runtime);
    Result RegisterAssetEditor(AssetEditorDescriptor descriptor,
                               const GameFeatures::RuntimeRegistry& runtime);
    Result RegisterViewportProvider(std::string id);
    void Freeze() noexcept { m_Schemas.Freeze(); m_Frozen = true; }
    bool Frozen() const noexcept { return m_Frozen; }
    const auto& ComponentEditors() const noexcept { return m_ComponentEditors; }
    const ComponentSchemaRegistry& ComponentSchemas() const noexcept { return m_Schemas; }
    const auto& AssetEditors() const noexcept { return m_AssetEditors; }
    const auto& ViewportProviders() const noexcept { return m_ViewportProviders; }
private:
    GameFeatures::FeatureDescriptor m_Feature;
    bool m_Frozen = false;
    ComponentSchemaRegistry m_Schemas;
    std::unordered_map<std::string, ComponentEditorDescriptor> m_ComponentEditors;
    std::unordered_map<std::string, AssetEditorDescriptor> m_AssetEditors;
    std::unordered_map<std::string, ViewportProviderDescriptor> m_ViewportProviders;
    std::unordered_map<std::string, std::string> m_DefaultEditorByType;
};
}
