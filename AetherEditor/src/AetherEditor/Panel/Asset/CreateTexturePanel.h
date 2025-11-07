#pragma once
#include <AetherEditor/Panel/Panel.h>
#include <Project/Asset/TextureAsset.h>
namespace AetherEditor::UI
{

class CreateTexturePanel : public Panel
{
public:
    void OnImGuiUpdate() override;

private:
    char m_FilePathBuffer[512]{};
    std::string m_Address;
    std::string m_AddressDisplay="Address: ";
    char m_NameBuffer[256]{};
    bool m_SRGB = true;
    std::string m_Folder;
public:
    void SetFolderAddress(const std::string& folderAddress)
    {
        m_Folder = folderAddress;
        m_Address = folderAddress+"/"+std::string(m_NameBuffer);
        m_AddressDisplay = "Address: " + m_Address;
    }
    Aether::Delegate<void(Aether::Project::ImportTextureParams&)> TextureCreateEventHandler;
};
} // namespace AetherEditor::UI