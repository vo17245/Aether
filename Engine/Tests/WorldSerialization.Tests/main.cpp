#include <World/Serialization/WorldArchive.h>

#include <fstream>
#include <iostream>
#include <stdexcept>

namespace
{
struct Counter { int value = 0; };
struct MoveOnly
{
    explicit MoveOnly(int v = 0) : value(std::make_unique<int>(v)) {}
    MoveOnly(MoveOnly&&) noexcept = default;
    MoveOnly& operator=(MoveOnly&&) noexcept = default;
    MoveOnly(const MoveOnly&) = delete;
    MoveOnly& operator=(const MoveOnly&) = delete;
    std::unique_ptr<int> value;
};
struct EntityLink { Aether::EntityId target = entt::null; };
struct Ephemeral { int value = 0; };
struct Unregistered { int value = 0; };
struct Blob { std::string path; };
struct CodecFailure { int value = 0; };
struct WriterFailure { int value = 0; };

void Check(bool value, const char* message)
{
    if (!value) throw std::runtime_error(message);
}

struct TempDirectory
{
    TempDirectory()
    {
        path = std::filesystem::temp_directory_path() / ("aether-world-serialization-" + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
        std::filesystem::create_directories(path);
    }
    ~TempDirectory() { std::error_code ignored; std::filesystem::remove_all(path, ignored); }
    std::filesystem::path path;
};
}

namespace Aether::Serialization
{
template<> struct ComponentSerialization<Counter>
{
    static constexpr std::string_view Type = "Test.Counter";
    static constexpr std::uint32_t Version = 1;
    static Result<Json> Serialize(const Counter& value, SaveContext&) { return Json{{"value", value.value}}; }
    static Result<Counter> Deserialize(const Json& json, std::uint32_t, LoadContext&)
    {
        if (!json.is_object() || !json.contains("value") || !json["value"].is_number_integer())
            return std::unexpected(ArchiveError{ArchiveErrorCode::InvalidFormat, "counter requires integer value"});
        return Counter{json["value"].get<int>()};
    }
};
template<> struct ComponentSerialization<MoveOnly>
{
    static constexpr std::string_view Type = "Test.MoveOnly";
    static constexpr std::uint32_t Version = 1;
    static Result<Json> Serialize(const MoveOnly& value, SaveContext&)
    {
        if (!value.value) return std::unexpected(ArchiveError{ArchiveErrorCode::InvalidArgument, "missing payload"});
        return Json{{"value", *value.value}};
    }
    static Result<MoveOnly> Deserialize(const Json& json, std::uint32_t, LoadContext&)
    { return MoveOnly{json.at("value").get<int>()}; }
};
template<> struct ComponentSerialization<EntityLink>
{
    static constexpr std::string_view Type = "Test.EntityLink";
    static constexpr std::uint32_t Version = 1;
    static Result<Json> Serialize(const EntityLink& value, SaveContext& context)
    {
        auto target = context.EncodeEntity(value.target);
        if (!target) return std::unexpected(target.error());
        return Json{{"target", std::move(*target)}};
    }
    static Result<EntityLink> Deserialize(const Json& json, std::uint32_t, LoadContext& context)
    {
        auto target = context.ResolveEntity(json.at("target"));
        if (!target) return std::unexpected(target.error());
        return EntityLink{target->value_or(entt::null)};
    }
};
template<> struct ComponentSerialization<Ephemeral>
{
    static constexpr std::string_view Type = "Test.Ephemeral";
    static constexpr std::uint32_t Version = 1;
};
template<> struct ComponentSerialization<Unregistered>
{
    static constexpr std::string_view Type = "Test.Unregistered";
    static constexpr std::uint32_t Version = 1;
};
template<> struct ComponentSerialization<Blob>
{
    static constexpr std::string_view Type = "Test.Blob";
    static constexpr std::uint32_t Version = 1;
    static Result<Json> Serialize(const Blob&, SaveContext& context)
    {
        auto asset = context.WriteAsset("Test.Png", ".png", [](const std::filesystem::path& path) -> Result<void> {
            const std::array<unsigned char, 8> bytes{137, 80, 78, 71, 13, 10, 26, 10};
            std::ofstream file(path, std::ios::binary);
            file.write(reinterpret_cast<const char*>(bytes.data()), bytes.size());
            if (!file) return std::unexpected(ArchiveError{ArchiveErrorCode::IoError, "cannot write fixture"});
            return {};
        });
        if (!asset) return std::unexpected(asset.error());
        return Json{{"asset", {{"path", asset->path}, {"kind", asset->kind}}}};
    }
    static Result<Blob> Deserialize(const Json& json, std::uint32_t, LoadContext& context)
    {
        AssetRef ref{json.at("asset").at("path").get<std::string>(), json.at("asset").at("kind").get<std::string>()};
        auto path = context.ResolveAsset(ref, "Test.Png");
        if (!path) return std::unexpected(path.error());
        return Blob{path->string()};
    }
};
template<> struct ComponentSerialization<CodecFailure>
{
    static constexpr std::string_view Type = "Test.ZCodecFailure";
    static constexpr std::uint32_t Version = 1;
    static Result<Json> Serialize(const CodecFailure&, SaveContext&)
    { return std::unexpected(ArchiveError{ArchiveErrorCode::CodecFailure, "intentional codec failure"}); }
    static Result<CodecFailure> Deserialize(const Json&, std::uint32_t, LoadContext&) { return CodecFailure{}; }
};
template<> struct ComponentSerialization<WriterFailure>
{
    static constexpr std::string_view Type = "Test.WriterFailure";
    static constexpr std::uint32_t Version = 1;
    static Result<Json> Serialize(const WriterFailure&, SaveContext& context)
    {
        auto asset = context.WriteAsset("Test.Partial", ".bin", [](const std::filesystem::path& path) -> Result<void> {
            std::ofstream file(path, std::ios::binary);
            file << "partial";
            file.flush();
            return std::unexpected(ArchiveError{ArchiveErrorCode::IoError, "intentional writer failure"});
        });
        if (!asset) return std::unexpected(asset.error());
        return Json::object();
    }
    static Result<WriterFailure> Deserialize(const Json&, std::uint32_t, LoadContext&) { return WriterFailure{}; }
};
}

int main()
{
    try
    {
        using namespace Aether;
        using namespace Aether::Serialization;
        ComponentCodecRegistry codecs;
        Check(bool(codecs.Register<Counter>()), "register counter");
        Check(bool(codecs.Register<MoveOnly>()), "register move-only");
        Check(bool(codecs.Register<EntityLink>()), "register entity link");
        Check(bool(codecs.RegisterTransient<Ephemeral>()), "register transient");
        Check(bool(codecs.Register<Blob>()), "register blob");
        Check(bool(codecs.Register<CodecFailure>()), "register failing codec");
        Check(bool(codecs.Register<WriterFailure>()), "register failing writer codec");
        Check(codecs.Register<Counter>().error().code == ArchiveErrorCode::DuplicateRegistration, "duplicate registration accepted");
        SaveContext noSourceRoot;
        auto implicitRelativeSource = noSourceRoot.CopyFile("relative.bin", "Test.Binary");
        Check(!implicitRelativeSource && implicitRelativeSource.error().code == ArchiveErrorCode::InvalidArgument,
              "relative source path silently used the process working directory");

        World source;
        const auto first = source.CreateEntity();
        const auto second = source.CreateEntity();
        source.AddComponent<Counter>(first, Counter{17});
        source.AddComponent<MoveOnly>(first, 41);
        source.AddComponent<EntityLink>(first, EntityLink{second});
        source.AddComponent<EntityLink>(second, EntityLink{first});
        source.AddComponent<Ephemeral>(second, Ephemeral{9});
        const auto nullReference = source.CreateEntity();
        source.AddComponent<EntityLink>(nullReference, EntityLink{entt::null});
        source.CreateEntity(); // An empty entity must survive.
        SaveContext saveContext;
        auto document = source.Serialize(codecs, saveContext);
        Check(bool(document), "memory serialization failed");
        LoadContext loadContext;
        auto restored = World::Deserialize(*document, codecs, loadContext);
        Check(bool(restored), "memory deserialization failed");
        auto counters = (*restored)->Select<Counter>();
        Check(counters.size() == 1 && counters.get<Counter>(*counters.begin()).value == 17, "counter round trip failed");
        auto moves = (*restored)->Select<MoveOnly>();
        Check(moves.size() == 1 && *moves.get<MoveOnly>(*moves.begin()).value == 41, "move-only round trip failed");
        auto links = (*restored)->Select<EntityLink>();
        Check(links.size() == 3, "entity links lost");
        bool sawNullReference = false;
        for (auto entity : links)
        {
            const auto target = links.get<EntityLink>(entity).target;
            if (target == entt::null) { sawNullReference = true; continue; }
            Check((*restored)->IsValid(target) && (*restored)->HasComponent<EntityLink>(target), "entity reference did not resolve");
            Check((*restored)->GetComponent<EntityLink>(target).target == entity, "cyclic reference changed");
        }
        Check(sawNullReference && (*restored)->Entities().size() == 4, "null reference or empty entity was lost");
        Check((*restored)->Select<Ephemeral>().size() == 0, "transient component was serialized");

        World unknown;
        unknown.AddComponent<Unregistered>(unknown.CreateEntity(), Unregistered{1});
        SaveContext unknownContext;
        auto unknownResult = unknown.Serialize(codecs, unknownContext);
        Check(!unknownResult && unknownResult.error().code == ArchiveErrorCode::UnknownComponent, "unregistered component was silently omitted");
        World destroyedUnknown;
        const auto destroyedEntity = destroyedUnknown.CreateEntity();
        destroyedUnknown.AddComponent<Unregistered>(destroyedEntity, Unregistered{2});
        destroyedUnknown.DestroyEntity(destroyedEntity);
        SaveContext destroyedContext;
        auto destroyedResult = destroyedUnknown.Serialize(codecs, destroyedContext);
        Check(bool(destroyedResult) && destroyedResult->at("entities").empty(),
              "empty storage left by destroying its entity was treated as an unknown component");

        const Json transientInput{{"entities", Json::array({Json{{"id", "e1"}, {"components", Json::array({
            Json{{"type", "Test.Ephemeral"}, {"version", 1}, {"data", Json::object()}}})}}})}};
        LoadContext transientInputContext;
        auto transientInputResult = World::Deserialize(transientInput, codecs, transientInputContext);
        Check(!transientInputResult && transientInputResult.error().code == ArchiveErrorCode::InvalidFormat,
              "transient component was accepted from a world document");

        TempDirectory temp;
        World packaged;
        auto blobEntity = packaged.CreateEntity();
        packaged.AddComponent<Blob>(blobEntity, Blob{});
        const auto target = temp.path / "world-a";
        auto saved = SaveWorldDirectory(packaged, codecs, Json{{"type", "Test"}}, target);
        Check(bool(saved), "directory save failed");
        auto noReplace = SaveWorldDirectory(packaged, codecs, Json{{"type", "Test"}}, target);
        Check(!noReplace && noReplace.error().code == ArchiveErrorCode::AlreadyExists, "existing target was replaced");
        const auto emptyTarget = temp.path / "empty-target";
        std::filesystem::create_directory(emptyTarget);
        auto noReplaceEmpty = SaveWorldDirectory(packaged, codecs, Json{{"type", "Test"}}, emptyTarget);
        Check(!noReplaceEmpty && noReplaceEmpty.error().code == ArchiveErrorCode::AlreadyExists
              && std::filesystem::is_empty(emptyTarget), "empty existing target was replaced");

        World codecFailureWorld;
        const auto codecFailureEntity = codecFailureWorld.CreateEntity();
        codecFailureWorld.AddComponent<Blob>(codecFailureEntity, Blob{});
        codecFailureWorld.AddComponent<CodecFailure>(codecFailureEntity, CodecFailure{});
        const auto codecFailureTarget = temp.path / "codec-failure";
        auto codecFailure = SaveWorldDirectory(codecFailureWorld, codecs, Json{{"type", "Test"}}, codecFailureTarget);
        Check(!codecFailure && codecFailure.error().code == ArchiveErrorCode::CodecFailure
              && !std::filesystem::exists(codecFailureTarget), "codec failure published a partial package");

        World writerFailureWorld;
        writerFailureWorld.AddComponent<WriterFailure>(writerFailureWorld.CreateEntity(), WriterFailure{});
        const auto writerFailureTarget = temp.path / "writer-failure";
        auto writerFailure = SaveWorldDirectory(writerFailureWorld, codecs, Json{{"type", "Test"}}, writerFailureTarget);
        Check(!writerFailure && writerFailure.error().code == ArchiveErrorCode::IoError
              && !std::filesystem::exists(writerFailureTarget), "asset writer failure published a partial package");
        const auto moved = temp.path / "world-b";
        std::filesystem::rename(target, moved);
        auto loaded = LoadWorldDirectory(moved, codecs);
        Check(bool(loaded), "moved directory load failed");
        auto blobs = loaded->world->Select<Blob>();
        Check(blobs.size() == 1, "asset-backed component was lost");
        std::ifstream png(blobs.get<Blob>(*blobs.begin()).path, std::ios::binary);
        std::array<unsigned char, 8> signature{};
        png.read(reinterpret_cast<char*>(signature.data()), signature.size());
        Check(signature[0] == 137 && signature[1] == 80 && signature[2] == 78, "attachment bytes changed");

        const auto missing = loaded->assets->Entries().front();
        std::filesystem::remove(moved / missing.path);
        auto missingLoad = LoadWorldDirectory(moved, codecs);
        Check(!missingLoad && missingLoad.error().code == ArchiveErrorCode::MissingAsset,
              "missing attachment was accepted");

        const auto duplicateKeys = temp.path / "duplicate-keys";
        std::filesystem::create_directory(duplicateKeys);
        {
            std::ofstream manifest(duplicateKeys / "world.json", std::ios::binary);
            manifest << R"({"format":"Aether.WorldDirectory","format":"Aether.WorldDirectory","version":1})";
        }
        auto duplicateKeyLoad = LoadWorldDirectory(duplicateKeys, codecs);
        Check(!duplicateKeyLoad && duplicateKeyLoad.error().code == ArchiveErrorCode::InvalidFormat,
              "duplicate manifest key was accepted");

        const auto deepManifest = temp.path / "deep-manifest";
        std::filesystem::create_directory(deepManifest);
        {
            std::ofstream manifest(deepManifest / "world.json", std::ios::binary);
            manifest << R"({"format":"Aether.WorldDirectory","version":1,"application":{"a":{"b":{"c":1}}},"assets":[],"entities":[]})";
        }
        DirectoryLoadOptions shallowLimits;
        shallowLimits.limits.maxJsonDepth = 2;
        auto deepLoad = LoadWorldDirectory(deepManifest, codecs, shallowLimits);
        Check(!deepLoad && deepLoad.error().code == ArchiveErrorCode::ResourceLimitExceeded,
              "manifest depth limit was not enforced during parsing");
        std::cout << "WorldSerializationTests passed\n";
        return 0;
    }
    catch (const std::exception& exception)
    {
        std::cerr << exception.what() << '\n';
        return 1;
    }
}
