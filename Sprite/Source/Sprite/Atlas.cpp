#include "Sprite/Atlas.h"
#include <Filesystem/Filesystem.h>
namespace Aether::Sprite
{

std::optional<AtlasInfo> AtlasInfo::Load(const std::string_view path)
{
    auto jsonStrOpt=Filesystem::ReadFileToString(path);
    if(!jsonStrOpt)
    {
        return std::nullopt;
    }
    auto json = Json::parse(*jsonStrOpt);
    auto infoEx = FromJson<AtlasInfo>{}(json);
    if (!infoEx)
    {
        return std::nullopt;
    
    }
    return infoEx.value();

}
bool AtlasInfo::Save(const std::string_view path)
{
    auto json = ToJson<AtlasInfo>{}(*this);
    auto jsonStr = json.dump(4);
    return Filesystem::WriteFile(path, jsonStr);
}

}
