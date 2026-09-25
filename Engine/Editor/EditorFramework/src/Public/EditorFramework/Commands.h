#pragma once

#include <EditorFramework/EditorComponents.h>
#include <GameFeature/ProjectWorld.h>

#include <functional>
#include <memory>
#include <optional>
#include <unordered_map>

namespace Aether::EditorFramework
{
class ComponentSchemaRegistry;
class AssetEditorRegistry;
enum class CommandMode { Apply, Undo, Redo };

struct EditOperation
{
    std::string commandId;
    Json payload;
};

struct CommandDescriptor
{
    using Apply = std::function<std::expected<EditOperation, std::string>(World&, const Json&)>;
    using MergeKey = std::function<std::optional<std::string>(const Json&)>;
    using MergePayload = std::function<std::expected<Json, std::string>(const Json&, const Json&)>;
    std::string id;
    Apply apply;
    MergeKey mergeKey;
    MergePayload mergePayload;
};

class CommandRegistry
{
public:
    std::expected<void, std::string> Register(CommandDescriptor descriptor);
    void Freeze() noexcept { m_Frozen = true; }
    bool Frozen() const noexcept { return m_Frozen; }
    const CommandDescriptor* Find(std::string_view id) const noexcept;
private:
    bool m_Frozen = false;
    std::unordered_map<std::string, CommandDescriptor> m_Commands;
};

struct HistoryAction
{
    EditOperation forward;
    EditOperation inverse;
};

struct HistoryEntry
{
    std::uint64_t beforeState = 0;
    std::uint64_t afterState = 0;
    std::vector<HistoryAction> actions;
    std::size_t memoryBytes = 0;
};

struct HistoryComponent
{
    std::vector<HistoryEntry> entries;
    std::size_t cursor = 0;
    std::uint64_t currentState = 0;
    std::uint64_t savedState = 0;
    std::uint64_t nextState = 1;
    std::size_t usedBytes = 0;
    std::size_t maxBytes = 32ull * 1024 * 1024;
    bool IsDirty() const noexcept { return currentState != savedState; }
};

struct EditorServicesComponent
{
    std::shared_ptr<const CommandRegistry> commands;
    std::shared_ptr<const GameFeatures::RuntimeRegistry> runtime;
    std::shared_ptr<const ProjectAssets::CatalogSnapshot> catalog;
    std::filesystem::path projectRoot;
    std::shared_ptr<const ComponentSchemaRegistry> componentSchemas;
    std::shared_ptr<const AssetEditorRegistry> assetEditors;
};

struct EditCommandRequestComponent
{
    GameFeatures::DocumentId documentId;
    GameFeatures::WorldInstanceId worldInstanceId;
    CommandMode mode = CommandMode::Apply;
    std::vector<EditOperation> operations;
    bool cancelled = false;
};

enum class EditRequestState { Pending, Applied, Rejected, Cancelled };
struct EditCommandResultComponent
{
    EditRequestState state = EditRequestState::Pending;
    std::string error;
    std::uint32_t operationsApplied = 0;
};

EntityId SubmitEditCommand(World& editorWorld, GameFeatures::DocumentId documentId,
    GameFeatures::WorldInstanceId worldInstanceId, std::vector<EditOperation> operations);
EntityId SubmitUndo(World& editorWorld, GameFeatures::DocumentId documentId,
    GameFeatures::WorldInstanceId worldInstanceId);
EntityId SubmitRedo(World& editorWorld, GameFeatures::DocumentId documentId,
    GameFeatures::WorldInstanceId worldInstanceId);
void CancelEditCommand(World& editorWorld, EntityId requestEntity);

// Must run at a main-thread safe point after all World dispatch calls have returned.
void ApplyEditCommands(World& editorWorld);
void MarkDocumentSaved(World& editorWorld, EntityId documentEntity);
std::expected<void, std::string> RegisterCoreEntityCommands(CommandRegistry& commands,
    std::shared_ptr<const GameFeatures::RuntimeRegistry> runtime);
}
