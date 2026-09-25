#include <EditorFramework/EditorFeatureRegistrar.h>

#include <ProjectAsset/Types.h>

namespace Aether::EditorFramework
{
namespace
{
std::expected<void, std::string> CheckFeature(const GameFeatures::FeatureDescriptor& feature, bool frozen)
{
    if (frozen) return std::unexpected("editor feature registrar is frozen");
    if (!ProjectAssets::IsStableIdentifier(feature.id)) return std::unexpected("feature ID is invalid");
    return {};
}
}

EditorFeatureRegistrar::Result EditorFeatureRegistrar::RegisterComponentEditor(
    ComponentEditorDescriptor descriptor, const GameFeatures::RuntimeRegistry& runtime)
{
    auto check = CheckFeature(m_Feature, m_Frozen);
    if (!check) return check;
    if (descriptor.ownerFeature != m_Feature.id)
        return std::unexpected("component editor owner differs from Editor Feature");
    const auto componentType = descriptor.componentType;
    if (m_ComponentEditors.contains(componentType))
        return std::unexpected("component editor is already registered: " + componentType);
    auto registered = m_Schemas.Register(descriptor, runtime);
    if (!registered) return registered;
    if (!m_ComponentEditors.emplace(componentType, std::move(descriptor)).second)
        return std::unexpected("component editor is already registered: " + componentType);
    return {};
}

EditorFeatureRegistrar::Result EditorFeatureRegistrar::RegisterAssetEditor(
    AssetEditorDescriptor descriptor, const GameFeatures::RuntimeRegistry& runtime)
{
    auto check = CheckFeature(m_Feature, m_Frozen);
    if (!check) return check;
    if (descriptor.ownerFeature != m_Feature.id)
        return std::unexpected("asset editor owner differs from Editor Feature");
    if (!ProjectAssets::IsStableIdentifier(descriptor.editorId) || descriptor.supportedTypes.empty()
        || !descriptor.createDocument)
        return std::unexpected("asset editor requires a stable ID, supported types, and a document factory");
    if (m_AssetEditors.contains(descriptor.editorId))
        return std::unexpected("asset editor ID is already registered: " + descriptor.editorId);
    std::unordered_set<std::string> uniqueTypes;
    for (const auto& assetType : descriptor.supportedTypes)
    {
        if (!ProjectAssets::IsStableIdentifier(assetType) || !uniqueTypes.emplace(assetType).second)
            return std::unexpected("asset editor has an invalid or duplicate supported type");
        const auto* assetDescriptor = runtime.AssetTypes().Find(assetType);
        if (!assetDescriptor) return std::unexpected("asset editor has no matching Runtime asset type: " + assetType);
        if (assetDescriptor->ownerFeature != m_Feature.id)
            return std::unexpected("asset editor owner differs from Runtime asset type owner");
        if (descriptor.isDefault && m_DefaultEditorByType.contains(assetType))
            return std::unexpected("default asset editor is already registered for: " + assetType);
    }
    const auto editorId = descriptor.editorId;
    m_AssetEditors.emplace(editorId, descriptor);
    if (descriptor.isDefault)
        for (const auto& assetType : descriptor.supportedTypes) m_DefaultEditorByType.emplace(assetType, editorId);
    return {};
}

EditorFeatureRegistrar::Result EditorFeatureRegistrar::RegisterViewportProvider(std::string id)
{
    auto check = CheckFeature(m_Feature, m_Frozen);
    if (!check) return check;
    if (!ProjectAssets::IsStableIdentifier(id)) return std::unexpected("viewport provider ID is invalid");
    if (!m_ViewportProviders.emplace(id, ViewportProviderDescriptor{m_Feature.id, id}).second)
        return std::unexpected("viewport provider ID is already registered: " + id);
    return {};
}
}
