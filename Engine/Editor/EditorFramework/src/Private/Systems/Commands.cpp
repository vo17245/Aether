#include <EditorFramework/Commands.h>
#include <EditorFramework/ComponentSchema.h>

#include <algorithm>
#include <unordered_set>

namespace Aether::EditorFramework
{
namespace
{
std::expected<EditOperation, std::string> Inverse(std::string id, Json payload)
{
    return EditOperation{std::move(id), std::move(payload)};
}

std::expected<std::string, std::string> ReadString(const Json& json, std::string_view key)
{
    if (!json.is_object() || !json.contains(key) || !json[key].is_string())
        return std::unexpected("command payload requires string field '" + std::string(key) + "'");
    return json[key].get<std::string>();
}

std::optional<EntityId> FindEntity(World& world, std::string_view persistentId)
{
    for (const auto entity : world.Select<GameFeatures::PersistentEntityIdComponent>())
        if (world.GetComponent<GameFeatures::PersistentEntityIdComponent>(entity).value.ToString() == persistentId)
            return entity;
    return std::nullopt;
}

std::expected<GameFeatures::PersistentEntityId, std::string> ReadPersistentId(const Json& json, std::string_view key)
{
    auto text = ReadString(json, key);
    if (!text) return std::unexpected(text.error());
    auto id = GameFeatures::PersistentEntityId::Parse(*text);
    if (!id) return std::unexpected("field '" + std::string(key) + "' is not a canonical UUID");
    return *id;
}

std::expected<Json, std::string> CaptureWorld(World& world, const GameFeatures::RuntimeRegistry& runtime)
{
    Serialization::SaveContext context;
    auto saved = world.Serialize(runtime.Codecs(), context);
    if (!saved) return std::unexpected(saved.error().message);
    return std::move(*saved);
}

std::expected<void, std::string> AddDescriptor(CommandRegistry& commands, std::string id, CommandDescriptor::Apply apply)
{
    return commands.Register({std::move(id), std::move(apply)});
}

std::size_t EntryMemory(const HistoryEntry& entry)
{
    std::size_t result = 0;
    for (const auto& action : entry.actions)
        result += action.forward.payload.dump().size() + action.inverse.payload.dump().size()
            + action.forward.commandId.size() + action.inverse.commandId.size();
    return result;
}

struct ProcessOutcome
{
    std::uint32_t operationsApplied = 0;
};

std::expected<ProcessOutcome, std::string> ProcessRequest(World& editorWorld,
    EditCommandRequestComponent& request, EditCommandResultComponent& result)
{
    EntityId servicesEntity = entt::null;
    for (const auto entity : editorWorld.Select<EditorServicesComponent>()) { servicesEntity = entity; break; }
    if (servicesEntity == entt::null)
        return std::unexpected("editor command services are not registered");
    const auto& services = editorWorld.GetComponent<EditorServicesComponent>(servicesEntity);
    if (!services.commands || !services.runtime || !services.catalog)
        return std::unexpected("editor command services are incomplete");

    for (const auto playEntity : editorWorld.Select<PlaySessionComponent>())
        if (editorWorld.GetComponent<PlaySessionComponent>(playEntity).state != PlayState::Stopped)
            return std::unexpected("persistent edits are locked while Play is active");

    EntityId documentEntity = entt::null;
    for (const auto entity : editorWorld.Select<DocumentComponent, WorldDocumentComponent, HistoryComponent>())
    {
        const auto& document = editorWorld.GetComponent<DocumentComponent>(entity);
        if (document.id == request.documentId && document.kind == DocumentKind::World)
        {
            documentEntity = entity;
            break;
        }
    }
    if (documentEntity == entt::null) return std::unexpected("target World document is closed or missing");
    auto& document = editorWorld.GetComponent<DocumentComponent>(documentEntity);
    auto& worldDocument = editorWorld.GetComponent<WorldDocumentComponent>(documentEntity);
    auto& history = editorWorld.GetComponent<HistoryComponent>(documentEntity);
    if (!worldDocument.authoringWorld || worldDocument.instanceId != request.worldInstanceId)
        return std::unexpected("edit request targets a stale World instance");
    if (request.mode == CommandMode::Apply && request.operations.empty())
        return ProcessOutcome{};
    if (request.mode != CommandMode::Apply && !request.operations.empty())
        return std::unexpected("Undo and Redo requests cannot contain edit operations");
    if (request.mode == CommandMode::Undo && history.cursor == 0)
        return std::unexpected("there is no edit to undo");
    if (request.mode == CommandMode::Redo && history.cursor >= history.entries.size())
        return std::unexpected("there is no edit to redo");

    auto snapshot = CaptureWorld(*worldDocument.authoringWorld, *services.runtime);
    if (!snapshot) return std::unexpected(snapshot.error());
    Serialization::LoadContext loadContext;
    auto detached = World::Deserialize(*snapshot, services.runtime->Codecs(), loadContext);
    if (!detached) return std::unexpected(detached.error().message);
    auto& candidate = **detached;

    HistoryComponent nextHistory = history;
    std::vector<HistoryAction> actions;
    std::uint32_t applied = 0;
    auto run = [&](const EditOperation& operation, bool captureInverse) -> std::expected<void, std::string> {
        if (request.cancelled) return std::unexpected("edit request was cancelled");
        if (operation.commandId == "aether.component.set-field")
        {
            auto componentType = ReadString(operation.payload, "componentType");
            auto fieldId = ReadString(operation.payload, "fieldId");
            if (!componentType) return std::unexpected(componentType.error());
            if (!fieldId) return std::unexpected(fieldId.error());
            if (!services.componentSchemas) return std::unexpected("component schema registry is unavailable");
            const auto* schema = services.componentSchemas->Find(*componentType);
            const auto* field = schema ? schema->FindField(*fieldId) : nullptr;
            if (!field) return std::unexpected("field has no registered Editor schema");
            if (field->kind == PropertyKind::AssetReference)
            {
                if (!operation.payload.contains("value")) return std::unexpected("asset reference value is missing");
                auto decoded = field->DecodeValue(operation.payload["value"]);
                if (!decoded) return std::unexpected(decoded.error());
                const auto& reference = std::get<AssetReferenceValue>(*decoded).value;
                if (reference)
                {
                    if (reference->project != services.catalog->Project())
                        return std::unexpected("asset reference belongs to another project");
                    const auto location = *componentType + "." + *fieldId;
                    auto resolved = ProjectAssets::ResolveAsset(*services.catalog, *reference,
                        field->expectedAssetType, services.projectRoot, services.runtime->AssetTypes(), location);
                    if (!resolved) return std::unexpected(resolved.error().message);
                    auto closure = ProjectAssets::ValidateReferenceClosure(*services.catalog, *reference,
                        services.projectRoot, services.runtime->AssetTypes(), location);
                    if (!closure) return std::unexpected(closure.error().message);
                }
            }
        }
        const auto* command = services.commands->Find(operation.commandId);
        if (!command) return std::unexpected("unknown edit command: " + operation.commandId);
        auto inverse = command->apply(candidate, operation.payload);
        if (!inverse) return std::unexpected(inverse.error());
        if (captureInverse) actions.push_back({operation, std::move(*inverse)});
        ++applied;
        return {};
    };

    if (request.mode == CommandMode::Apply)
    {
        for (const auto& operation : request.operations)
        {
            auto appliedOperation = run(operation, true);
            if (!appliedOperation) return std::unexpected(appliedOperation.error());
        }
        if (request.cancelled) return std::unexpected("edit request was cancelled");
        if (!actions.empty())
        {
            if (nextHistory.cursor < nextHistory.entries.size())
            {
                for (std::size_t i = nextHistory.cursor; i < nextHistory.entries.size(); ++i)
                    nextHistory.usedBytes -= std::min(nextHistory.usedBytes, nextHistory.entries[i].memoryBytes);
                nextHistory.entries.erase(nextHistory.entries.begin() + static_cast<std::ptrdiff_t>(nextHistory.cursor), nextHistory.entries.end());
            }
            bool merged = false;
            if (actions.size() == 1 && nextHistory.cursor == nextHistory.entries.size()
                && !nextHistory.entries.empty() && nextHistory.currentState != nextHistory.savedState)
            {
                auto& previous = nextHistory.entries.back();
                auto& action = actions.front();
                const auto* descriptor = services.commands->Find(action.forward.commandId);
                if (previous.actions.size() == 1 && descriptor && descriptor->mergeKey && descriptor->mergePayload
                    && previous.actions.front().forward.commandId == action.forward.commandId)
                {
                    const auto previousKey = descriptor->mergeKey(previous.actions.front().forward.payload);
                    const auto currentKey = descriptor->mergeKey(action.forward.payload);
                    if (previousKey && currentKey && *previousKey == *currentKey)
                    {
                        auto mergedPayload = descriptor->mergePayload(previous.actions.front().forward.payload, action.forward.payload);
                        if (mergedPayload)
                        {
                            nextHistory.usedBytes -= previous.memoryBytes;
                            previous.actions.front().forward.payload = std::move(*mergedPayload);
                            previous.afterState = nextHistory.nextState++;
                            previous.memoryBytes = EntryMemory(previous);
                            nextHistory.usedBytes += previous.memoryBytes;
                            nextHistory.currentState = previous.afterState;
                            merged = true;
                        }
                    }
                }
            }
            if (!merged)
            {
                HistoryEntry entry;
                entry.beforeState = nextHistory.currentState;
                entry.afterState = nextHistory.nextState++;
                entry.actions = std::move(actions);
                entry.memoryBytes = EntryMemory(entry);
                nextHistory.usedBytes += entry.memoryBytes;
                nextHistory.entries.push_back(std::move(entry));
                nextHistory.currentState = nextHistory.entries.back().afterState;
            }
            nextHistory.cursor = nextHistory.entries.size();
            while (nextHistory.usedBytes > nextHistory.maxBytes && !nextHistory.entries.empty())
            {
                nextHistory.usedBytes -= nextHistory.entries.front().memoryBytes;
                nextHistory.entries.erase(nextHistory.entries.begin());
                if (nextHistory.cursor > 0) --nextHistory.cursor;
            }
        }
    }
    else if (request.mode == CommandMode::Undo)
    {
        const auto& entry = nextHistory.entries[nextHistory.cursor - 1];
        for (auto it = entry.actions.rbegin(); it != entry.actions.rend(); ++it)
        {
            auto appliedOperation = run(it->inverse, false);
            if (!appliedOperation) return std::unexpected(appliedOperation.error());
        }
        --nextHistory.cursor;
        nextHistory.currentState = entry.beforeState;
    }
    else
    {
        const auto& entry = nextHistory.entries[nextHistory.cursor];
        for (const auto& action : entry.actions)
        {
            auto appliedOperation = run(action.forward, false);
            if (!appliedOperation) return std::unexpected(appliedOperation.error());
        }
        ++nextHistory.cursor;
        nextHistory.currentState = entry.afterState;
    }

    auto validated = GameFeatures::EncodeProjectWorld(candidate, request.documentId, *services.runtime,
        *services.catalog, services.projectRoot);
    if (!validated) return std::unexpected(validated.error().message);

    // All fallible preparation is complete. The registry swap and history move are
    // non-allocating safe-point commits; World notifies mounted systems afterward.
    worldDocument.authoringWorld->ReplaceDataFrom(candidate);
    history = std::move(nextHistory);
    document.dirty = history.IsDirty();
    result.operationsApplied = applied;
    result.state = EditRequestState::Applied;
    result.error.clear();
    return ProcessOutcome{applied};
}
}

std::expected<void, std::string> CommandRegistry::Register(CommandDescriptor descriptor)
{
    if (m_Frozen) return std::unexpected("command registry is frozen");
    if (!ProjectAssets::IsStableIdentifier(descriptor.id) || !descriptor.apply)
        return std::unexpected("command requires a stable ID and apply callback");
    const auto id = descriptor.id;
    if (!m_Commands.emplace(id, std::move(descriptor)).second)
        return std::unexpected("edit command is already registered");
    return {};
}

const CommandDescriptor* CommandRegistry::Find(std::string_view id) const noexcept
{
    const auto it = m_Commands.find(std::string(id));
    return it == m_Commands.end() ? nullptr : &it->second;
}

EntityId SubmitEditCommand(World& editorWorld, GameFeatures::DocumentId documentId,
    GameFeatures::WorldInstanceId worldInstanceId, std::vector<EditOperation> operations)
{
    const auto entity = editorWorld.CreateEntity();
    editorWorld.AddComponent<EditCommandRequestComponent>(entity,
        EditCommandRequestComponent{documentId, worldInstanceId, CommandMode::Apply, std::move(operations), false});
    editorWorld.AddComponent<EditCommandResultComponent>(entity);
    return entity;
}

EntityId SubmitUndo(World& editorWorld, GameFeatures::DocumentId documentId, GameFeatures::WorldInstanceId worldInstanceId)
{
    const auto entity = editorWorld.CreateEntity();
    editorWorld.AddComponent<EditCommandRequestComponent>(entity, EditCommandRequestComponent{documentId, worldInstanceId, CommandMode::Undo, {}, false});
    editorWorld.AddComponent<EditCommandResultComponent>(entity);
    return entity;
}

EntityId SubmitRedo(World& editorWorld, GameFeatures::DocumentId documentId, GameFeatures::WorldInstanceId worldInstanceId)
{
    const auto entity = editorWorld.CreateEntity();
    editorWorld.AddComponent<EditCommandRequestComponent>(entity, EditCommandRequestComponent{documentId, worldInstanceId, CommandMode::Redo, {}, false});
    editorWorld.AddComponent<EditCommandResultComponent>(entity);
    return entity;
}

void CancelEditCommand(World& editorWorld, EntityId requestEntity)
{
    if (editorWorld.IsValid(requestEntity) && editorWorld.HasComponent<EditCommandRequestComponent>(requestEntity))
        editorWorld.GetComponent<EditCommandRequestComponent>(requestEntity).cancelled = true;
}

void ApplyEditCommands(World& editorWorld)
{
    std::vector<EntityId> requests;
    for (const auto entity : editorWorld.Select<EditCommandRequestComponent>()) requests.push_back(entity);
    for (const auto entity : requests)
    {
        if (!editorWorld.IsValid(entity) || !editorWorld.HasComponent<EditCommandRequestComponent>(entity)) continue;
        auto& request = editorWorld.GetComponent<EditCommandRequestComponent>(entity);
        if (!editorWorld.HasComponent<EditCommandResultComponent>(entity))
            editorWorld.AddComponent<EditCommandResultComponent>(entity);
        auto& result = editorWorld.GetComponent<EditCommandResultComponent>(entity);
        if (request.cancelled)
        {
            result.state = EditRequestState::Cancelled;
            result.error = "edit request was cancelled";
        }
        else
        {
            try
            {
                auto outcome = ProcessRequest(editorWorld, request, result);
                if (!outcome)
                {
                    result.state = EditRequestState::Rejected;
                    result.error = outcome.error();
                }
            }
            catch (const std::exception& exception)
            {
                result.state = EditRequestState::Rejected;
                result.error = exception.what();
            }
            catch (...)
            {
                result.state = EditRequestState::Rejected;
                result.error = "unknown exception while applying edit request";
            }
        }
        editorWorld.RemoveComponent<EditCommandRequestComponent>(entity);
    }
}

void MarkDocumentSaved(World& editorWorld, EntityId documentEntity)
{
    if (!editorWorld.IsValid(documentEntity) || !editorWorld.HasComponent<HistoryComponent>(documentEntity)) return;
    auto& history = editorWorld.GetComponent<HistoryComponent>(documentEntity);
    history.savedState = history.currentState;
    if (editorWorld.HasComponent<DocumentComponent>(documentEntity))
        editorWorld.GetComponent<DocumentComponent>(documentEntity).dirty = false;
}

std::expected<void, std::string> RegisterCoreEntityCommands(CommandRegistry& commands,
    std::shared_ptr<const GameFeatures::RuntimeRegistry> runtime)
{
    if (!runtime) return std::unexpected("core entity commands require a Runtime registry");

    auto created = AddDescriptor(commands, "aether.entity.create", [](World& world, const Json& payload) -> std::expected<EditOperation, std::string> {
        auto id = ReadPersistentId(payload, "entityId");
        auto name = ReadString(payload, "name");
        if (!id) return std::unexpected(id.error());
        if (!name) return std::unexpected(name.error());
        if (FindEntity(world, id->ToString())) return std::unexpected("persistent entity ID already exists");
        const auto entity = world.CreateEntity();
        world.AddComponent<GameFeatures::PersistentEntityIdComponent>(entity, GameFeatures::PersistentEntityIdComponent{*id});
        world.AddComponent<GameFeatures::EntityNameComponent>(entity, GameFeatures::EntityNameComponent{*name});
        return Inverse("aether.entity.delete-subtree", {{"entityId", id->ToString()}});
    });
    if (!created) return created;

    CommandDescriptor renameDescriptor;
    renameDescriptor.id = "aether.entity.rename";
    renameDescriptor.apply = [](World& world, const Json& payload) -> std::expected<EditOperation, std::string> {
        auto id = ReadPersistentId(payload, "entityId");
        auto name = ReadString(payload, "name");
        if (!id) return std::unexpected(id.error());
        if (!name) return std::unexpected(name.error());
        auto entity = FindEntity(world, id->ToString());
        if (!entity) return std::unexpected("entity does not exist");
        std::string oldName;
        if (world.HasComponent<GameFeatures::EntityNameComponent>(*entity))
        {
            auto& component = world.GetComponent<GameFeatures::EntityNameComponent>(*entity);
            oldName = std::move(component.value);
            component.value = *name;
        }
        else world.AddComponent<GameFeatures::EntityNameComponent>(*entity, GameFeatures::EntityNameComponent{*name});
        return Inverse("aether.entity.rename", {{"entityId", id->ToString()}, {"name", std::move(oldName)}});
    };
    renameDescriptor.mergeKey = [](const Json& payload) -> std::optional<std::string> {
        if (!payload.is_object() || !payload.contains("entityId") || !payload["entityId"].is_string()) return std::nullopt;
        return payload["entityId"].get<std::string>();
    };
    renameDescriptor.mergePayload = [](const Json& oldPayload, const Json& newPayload) -> std::expected<Json, std::string> {
        if (!oldPayload.is_object() || !newPayload.is_object() || !oldPayload.contains("entityId") || !newPayload.contains("entityId")
            || oldPayload["entityId"] != newPayload["entityId"] || !newPayload.contains("name") || !newPayload["name"].is_string())
            return std::unexpected("rename edits cannot be merged");
        return newPayload;
    };
    auto renamed = commands.Register(std::move(renameDescriptor));
    if (!renamed) return renamed;

    auto reparent = AddDescriptor(commands, "aether.entity.reparent", [](World& world, const Json& payload) -> std::expected<EditOperation, std::string> {
        auto childId = ReadPersistentId(payload, "entityId");
        if (!childId) return std::unexpected(childId.error());
        auto child = FindEntity(world, childId->ToString());
        if (!child) return std::unexpected("entity does not exist");
        if (!payload.is_object() || !payload.contains("parentId") || !(payload["parentId"].is_null() || payload["parentId"].is_string()))
            return std::unexpected("reparent command requires string or null parentId");
        Json oldParent = nullptr;
        if (world.HasComponent<GameFeatures::ParentComponent>(*child))
            oldParent = world.GetComponent<GameFeatures::ParentComponent>(*child).parent.ToString();
        if (payload["parentId"].is_null())
        {
            if (world.HasComponent<GameFeatures::ParentComponent>(*child)) world.RemoveComponent<GameFeatures::ParentComponent>(*child);
        }
        else
        {
            auto newParent = GameFeatures::PersistentEntityId::Parse(payload["parentId"].get<std::string>());
            if (!newParent) return std::unexpected("parentId is not a canonical UUID");
            if (*newParent == childId) return std::unexpected("entity cannot be its own parent");
            if (!FindEntity(world, newParent->ToString())) return std::unexpected("parent entity does not exist");
            auto cursor = *newParent;
            std::unordered_set<std::string> visited;
            while (visited.emplace(cursor.ToString()).second)
            {
                if (cursor == childId) return std::unexpected("reparent would create a hierarchy cycle");
                auto ancestorEntity = FindEntity(world, cursor.ToString());
                if (!ancestorEntity || !world.HasComponent<GameFeatures::ParentComponent>(*ancestorEntity)) break;
                cursor = world.GetComponent<GameFeatures::ParentComponent>(*ancestorEntity).parent;
            }
            if (world.HasComponent<GameFeatures::ParentComponent>(*child))
                world.GetComponent<GameFeatures::ParentComponent>(*child).parent = *newParent;
            else world.AddComponent<GameFeatures::ParentComponent>(*child, GameFeatures::ParentComponent{*newParent});
        }
        return Inverse("aether.entity.reparent", {{"entityId", childId->ToString()}, {"parentId", std::move(oldParent)}});
    });
    if (!reparent) return reparent;

    auto restore = AddDescriptor(commands, "aether.restore-world-state", [runtime](World& world, const Json& payload) -> std::expected<EditOperation, std::string> {
        if (!payload.is_object() || !payload.contains("archive")) return std::unexpected("restore command requires archive payload");
        auto current = CaptureWorld(world, *runtime);
        if (!current) return std::unexpected(current.error());
        Serialization::LoadContext context;
        auto replacement = World::Deserialize(payload["archive"], runtime->Codecs(), context);
        if (!replacement) return std::unexpected(replacement.error().message);
        world.ReplaceDataFrom(**replacement);
        return Inverse("aether.restore-world-state", {{"archive", std::move(*current)}});
    });
    if (!restore) return restore;

    auto eraseSubtree = AddDescriptor(commands, "aether.entity.delete-subtree", [runtime](World& world, const Json& payload) -> std::expected<EditOperation, std::string> {
        auto rootId = ReadPersistentId(payload, "entityId");
        if (!rootId) return std::unexpected(rootId.error());
        if (!FindEntity(world, rootId->ToString())) return std::unexpected("entity does not exist");
        std::unordered_set<std::string> subtree{rootId->ToString()};
        bool grew = true;
        while (grew)
        {
            grew = false;
            for (const auto entity : world.Select<GameFeatures::PersistentEntityIdComponent, GameFeatures::ParentComponent>())
            {
                const auto& childId = world.GetComponent<GameFeatures::PersistentEntityIdComponent>(entity).value;
                const auto& parentId = world.GetComponent<GameFeatures::ParentComponent>(entity).parent;
                if (subtree.contains(parentId.ToString()) && subtree.emplace(childId.ToString()).second) grew = true;
            }
        }
        for (const auto entity : world.Entities())
        {
            if (!world.HasComponent<GameFeatures::PersistentEntityIdComponent>(entity)) continue;
            const auto& owner = world.GetComponent<GameFeatures::PersistentEntityIdComponent>(entity).value;
            if (subtree.contains(owner.ToString())) continue;
            for (const auto& [type, descriptor] : runtime->Components())
            {
                (void)type;
                if (!descriptor.hasComponent || !descriptor.visitEntityReferences || !descriptor.hasComponent(world, entity)) continue;
                bool dangling = false;
                descriptor.visitEntityReferences(world, entity, [&](const GameFeatures::PersistentEntityId& target) {
                    if (subtree.contains(target.ToString())) dangling = true;
                });
                if (dangling) return std::unexpected("subtree is referenced by an entity outside the subtree");
            }
        }
        auto snapshot = CaptureWorld(world, *runtime);
        if (!snapshot) return std::unexpected(snapshot.error());
        std::vector<EntityId> targets;
        for (const auto entity : world.Entities())
            if (world.HasComponent<GameFeatures::PersistentEntityIdComponent>(entity)
                && subtree.contains(world.GetComponent<GameFeatures::PersistentEntityIdComponent>(entity).value.ToString()))
                targets.push_back(entity);
        for (const auto entity : targets) world.DestroyEntity(entity);
        return Inverse("aether.restore-world-state", {{"archive", std::move(*snapshot)}});
    });
    if (!eraseSubtree) return eraseSubtree;
    return {};
}
}
