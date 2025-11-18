#include "TextureViewPanel.h"
#include <Async/GlobalThreadPool.h>
#include <AetherEditor/Global/GlobalProject.h>
#include <AetherEditor/Utils/LoadTexture.h>
#include <Project/Asset/TextureAsset.h>
namespace AetherEditor::UI
{
std::optional<std::string> TextureViewPanel::SetImageAddress(const std::string& address)
{
    std::string path = GlobalProject::GetProject()->GetContentDirPath() + address;
    auto contentOpt = GlobalProject::GetProject()->GetContent(address);
    if (!contentOpt)
    {
        return "Content not found at address: " + address;
    }
    auto* contentPtr = *contentOpt;
    if (!contentPtr)
    {
        return "Content not found at address: " + address;
    }
    auto& content = *contentPtr;
    if (content.GetType() != Project::ContentTreeNodeType::Asset)
    {
        return "Content at address is not an asset: " + address;
    }
    auto& assetContent = static_cast<Project::AssetContentNode&>(content);
    auto* asset = assetContent.GetAsset();
    if (!asset)
    {
        return "Asset pointer is null at address: " + address;
    }
    if (asset->GetType() != Project::AssetType::Texture)
    {
        return "Asset at address is not a texture: " + address;
    }
    auto& textureAsset = static_cast<Project::TextureAsset&>(*asset);

    GlobalThreadPool::Enqueue(
        [path, sRGB = textureAsset.sRGB]() -> std::expected<DeviceTexture, std::string> {
            if (sRGB)
            {
                return Utils::LoadSrgbTexture(path);
            }
            else
            {
                return Utils::LoadLinearTexture(path);
            }
        },
        [this](std::expected<DeviceTexture, std::string>&& result) {
            if (!result)
            {
                m_ErrorMessage = result.error();
                m_Image.reset();
                m_ImageView.SetImage(nullptr);
                return;
            }
            m_Texture = CreateRef<DeviceTexture>(std::move(result.value()));
            auto imgOpt = ImGuiComponent::Image::Create(m_Texture);
            if (!imgOpt)
            {
                m_ErrorMessage = imgOpt.error();
                m_Image.reset();
                m_ImageView.SetImage(nullptr);
                return;
            }
            m_Image = CreateScope<ImGuiComponent::Image>(std::move(imgOpt.value()));
            m_ErrorMessage.reset();
            m_ImageView.SetImage(m_Image.get());
        });
    return std::nullopt;
}

void TextureViewPanel::OnImGuiUpdate()
{
    m_ImageView.Draw();
}
} // namespace AetherEditor::UI