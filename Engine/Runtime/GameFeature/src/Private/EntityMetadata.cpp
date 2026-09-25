#include <GameFeature/EntityMetadata.h>

namespace Aether::GameFeatures
{
Serialization::Result<void> RegisterEntityMetadata(Serialization::ComponentCodecRegistry& codecs)
{
    auto persistent = codecs.Register<PersistentEntityIdComponent>();
    if (!persistent) return persistent;
    auto name = codecs.Register<EntityNameComponent>();
    if (!name) return name;
    auto parent = codecs.Register<ParentComponent>();
    if (!parent) return parent;
    return codecs.RegisterTransient<Serialization::ExcludeFromArchiveComponent>();
}
}
