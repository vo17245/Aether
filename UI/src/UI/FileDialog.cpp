#include "FileDialog.h"
#include "nfd.h"
namespace Aether::UI
{
std::expected<std::string, std::string> SyncSelectFile()
{
    nfdchar_t* outPath = NULL;
    nfdresult_t result = NFD_OpenDialog(NULL, NULL, &outPath);

    if (result == NFD_OKAY)
    {
        std::string path(outPath);
        free(outPath);
        return path;
    }
    else if (result == NFD_CANCEL)
    {
        return std::unexpected("user cancel select");
    }
    else
    {
        return std::unexpected(NFD_GetError());
    }
}
std::expected<std::string,std::string> SyncSelectDirectroy()
{
    nfdchar_t* outPath = NULL;
    nfdresult_t result = NFD_PickFolder( NULL, &outPath);

    if (result == NFD_OKAY)
    {
        std::string path(outPath);
        free(outPath);
        return path;
    }
    else if (result == NFD_CANCEL)
    {
        return std::unexpected("user cancel select");
    }
    else
    {
        return std::unexpected(NFD_GetError());
    }
}
} // namespace Aether::UI