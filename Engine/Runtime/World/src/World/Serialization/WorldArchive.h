#pragma once

#include <World/Serialization/ComponentCodecRegistry.h>

namespace Aether::Serialization
{
// Synchronous Save As. The target must not exist; keep the World and its source
// files unchanged until the operation returns.
Result<void> SaveWorldDirectory(const World& world, const ComponentCodecRegistry& codecs,
                                const Json& application, const std::filesystem::path& target,
                                const DirectorySaveOptions& options = {});

// Returns a detached World without Systems. Keep the package directory readable
// while the returned World streams packaged assets.
Result<LoadedWorldDirectory> LoadWorldDirectory(const std::filesystem::path& source,
                                                const ComponentCodecRegistry& codecs,
                                                const DirectoryLoadOptions& options = {});
} // namespace Aether::Serialization
