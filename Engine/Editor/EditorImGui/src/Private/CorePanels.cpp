#include <EditorImGui/CorePanels.h>

#include <EditorFramework/AssetEditor.h>
#include <EditorFramework/ComponentSchema.h>
#include <EditorFramework/Commands.h>
#include <EditorFramework/EditorComponents.h>
#include <GameFeature/EntityMetadata.h>
#include <ImGui/Core/imgui.h>

#include <algorithm>
#include <array>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

namespace Aether::EditorImGui
{
namespace
{
using namespace EditorFramework;

const EditorServicesComponent* FindServices(const World& world)
{
    for (const auto entity : world.Select<EditorServicesComponent>())
        return &world.GetComponent<EditorServicesComponent>(entity);
    return nullptr;
}

const ProjectStateComponent* FindProject(const World& world)
{
    for (const auto entity : world.Select<ProjectStateComponent>())
        return &world.GetComponent<ProjectStateComponent>(entity);
    return nullptr;
}

void SubmitDocumentIntent(UiIntentSink& intents, std::string type,
                          const GameFeatures::DocumentId& document, Json payload = Json::object())
{
    payload["documentId"] = document.ToString();
    intents.Submit(std::move(type), std::move(payload));
}

void SubmitEdit(UiIntentSink& intents, const DocumentComponent& document,
                const WorldDocumentComponent& worldDocument, std::string command, Json payload)
{
    intents.Submit("editor.edit.apply", Json{
        {"documentId", document.id.ToString()},
        {"worldInstanceId", worldDocument.instanceId.ToString()},
        {"operations", Json::array({Json{{"commandId", std::move(command)}, {"payload", std::move(payload)}}})},
    });
}

struct ActiveWorld
{
    EntityId entity = entt::null;
    const DocumentComponent* document = nullptr;
    const WorldDocumentComponent* worldDocument = nullptr;
    const World* world = nullptr;
    const GameFeatures::PersistentEntityId* selectedEntity = nullptr;
};

EntityId FindPersistentEntity(const World& world, const GameFeatures::PersistentEntityId& id)
{
    for (const auto candidate : world.Select<GameFeatures::PersistentEntityIdComponent>())
        if (world.GetComponent<GameFeatures::PersistentEntityIdComponent>(candidate).value == id)
            return candidate;
    return entt::null;
}

std::optional<ActiveWorld> FindActiveWorld(const World& editorWorld)
{
    const SelectionComponent* selection = nullptr;
    for (const auto entity : editorWorld.Select<SelectionComponent>())
    {
        const auto& candidate = editorWorld.GetComponent<SelectionComponent>(entity);
        if (candidate.entity) { selection = &candidate; break; }
        if (!selection) selection = &candidate;
    }
    if (!selection) return std::nullopt;
    for (const auto entity : editorWorld.Select<DocumentComponent, WorldDocumentComponent>())
    {
        const auto& document = editorWorld.GetComponent<DocumentComponent>(entity);
        const auto& worldDocument = editorWorld.GetComponent<WorldDocumentComponent>(entity);
        if (document.kind != DocumentKind::World || worldDocument.instanceId != selection->world ||
            !worldDocument.authoringWorld)
            continue;
        return ActiveWorld{entity, &document, &worldDocument, worldDocument.authoringWorld.get(),
                           selection->entity ? &*selection->entity : nullptr};
    }
    return std::nullopt;
}

void DrawDocumentTabs(const World& world, UiIntentSink& intents)
{
    if (!ImGui::Begin("Documents")) { ImGui::End(); return; }
    if (ImGui::Button("New World"))
        intents.Submit("editor.document.new-world", Json::object());
    static char worldPath[1024]{};
    ImGui::InputText("World file", worldPath, sizeof(worldPath));
    if (ImGui::Button("Open World") && worldPath[0] != '\0')
        intents.Submit("editor.document.open-world", {{"path", worldPath}});
    static std::string pendingClose;
    for (const auto entity : world.Select<DocumentComponent>())
    {
        const auto& document = world.GetComponent<DocumentComponent>(entity);
        const char* kind = document.kind == DocumentKind::World ? "World" : "Asset";
        ImGui::PushID(document.id.ToString().c_str());
        const auto label = std::string(kind) + " " + document.id.ToString().substr(0, 8) +
                           (document.dirty ? " *" : "");
        if (ImGui::Selectable(label.c_str()))
            SubmitDocumentIntent(intents, "editor.document.focus", document.id);
        ImGui::SameLine();
        if (ImGui::SmallButton("Close"))
        {
            if (document.dirty)
            {
                pendingClose = document.id.ToString();
                ImGui::OpenPopup("Close unsaved document");
            }
            else
                SubmitDocumentIntent(intents, "editor.document.close",
                                     document.id, {{"resolution", "discard"}});
        }
        ImGui::PopID();
        if (!document.error.empty())
            ImGui::TextWrapped("%s", document.error.c_str());
    }
    if (ImGui::BeginPopupModal("Close unsaved document", nullptr,
                               ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::TextUnformatted("This document has unsaved changes.");
        if (ImGui::Button("Save and Close"))
        {
            intents.Submit("editor.document.close", {{"documentId", pendingClose}, {"resolution", "save"}});
            pendingClose.clear();
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Discard Changes"))
        {
            intents.Submit("editor.document.close", {{"documentId", pendingClose}, {"resolution", "discard"}});
            pendingClose.clear();
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel"))
        {
            pendingClose.clear();
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
    ImGui::End();
}

void DrawHierarchy(const World& editorWorld, UiIntentSink& intents)
{
    if (!ImGui::Begin("Hierarchy")) { ImGui::End(); return; }
    const auto active = FindActiveWorld(editorWorld);
    if (!active)
    {
        ImGui::TextUnformatted("Open a World document to view its entities.");
        ImGui::End();
        return;
    }
    if (ImGui::Button("Create Entity"))
        SubmitEdit(intents, *active->document, *active->worldDocument, "aether.entity.create",
            {{"entityId", GameFeatures::PersistentEntityId::Create().ToString()}, {"name", "Entity"}});

    const auto entries = active->world->Select<GameFeatures::PersistentEntityIdComponent>();
    std::vector<EntityId> entities(entries.begin(), entries.end());
    for (const auto entity : entities)
    {
        const auto& id = active->world->GetComponent<GameFeatures::PersistentEntityIdComponent>(entity).value;
        const auto label = active->world->HasComponent<GameFeatures::EntityNameComponent>(entity)
            ? active->world->GetComponent<GameFeatures::EntityNameComponent>(entity).value
            : std::string("Unnamed Entity");
        std::string visible = label;
        if (active->world->HasComponent<GameFeatures::ParentComponent>(entity))
        {
            const auto& parent = active->world->GetComponent<GameFeatures::ParentComponent>(entity).parent;
            visible += "  (child of " + parent.ToString().substr(0, 8) + ")";
        }
        ImGui::PushID(id.ToString().c_str());
        const bool selected = active->selectedEntity && *active->selectedEntity == id;
        if (ImGui::Selectable(visible.c_str(), selected))
            intents.Submit("editor.selection.set", {{"documentId", active->document->id.ToString()},
                {"worldInstanceId", active->worldDocument->instanceId.ToString()}, {"entityId", id.ToString()}});
        if (ImGui::BeginDragDropSource())
        {
            const auto payload = id.ToString();
            ImGui::SetDragDropPayload("AETHER_ENTITY_ID", payload.c_str(), payload.size() + 1);
            ImGui::TextUnformatted(label.c_str());
            ImGui::EndDragDropSource();
        }
        if (ImGui::BeginDragDropTarget())
        {
            if (const auto* payload = ImGui::AcceptDragDropPayload("AETHER_ENTITY_ID"))
            {
                const std::string child(static_cast<const char*>(payload->Data));
                if (child != id.ToString())
                    SubmitEdit(intents, *active->document, *active->worldDocument, "aether.entity.reparent",
                        {{"entityId", child}, {"parentId", id.ToString()}});
            }
            ImGui::EndDragDropTarget();
        }
        ImGui::SameLine();
        if (ImGui::SmallButton("Delete"))
            SubmitEdit(intents, *active->document, *active->worldDocument, "aether.entity.delete-subtree",
                {{"entityId", id.ToString()}});
        ImGui::PopID();
    }
    ImGui::End();
}

std::optional<PropertyValue> DrawProperty(const FieldDescriptor& field,
                                          const InspectorFieldModel& model,
                                          const ProjectAssets::CatalogSnapshot& catalog,
                                          UiIntentSink& intents)
{
    if (!model.value || model.readOnly) return std::nullopt;
    auto changed = std::optional<PropertyValue>{};
    ImGui::PushID(field.id.c_str());
    switch (field.kind)
    {
    case PropertyKind::Boolean:
    {
        bool value = std::get<bool>(*model.value);
        if (ImGui::Checkbox(field.label.c_str(), &value)) changed = value;
        break;
    }
    case PropertyKind::Integer:
    {
        auto value = std::get<std::int64_t>(*model.value);
        if (ImGui::InputScalar(field.label.c_str(), ImGuiDataType_S64, &value)) changed = value;
        break;
    }
    case PropertyKind::Float:
    {
        float value = static_cast<float>(std::get<double>(*model.value));
        const float minimum = field.minimum ? static_cast<float>(*field.minimum) : 0.0f;
        const float maximum = field.maximum ? static_cast<float>(*field.maximum) : 0.0f;
        if (ImGui::DragFloat(field.label.c_str(), &value, 0.1f,
                             field.minimum ? minimum : 0.0f, field.maximum ? maximum : 0.0f))
            changed = static_cast<double>(value);
        break;
    }
    case PropertyKind::String:
    {
        auto value = std::get<std::string>(*model.value);
        std::vector<char> buffer(std::max<std::size_t>(value.size() + 128, 256), '\0');
        std::memcpy(buffer.data(), value.data(), value.size());
        if (ImGui::InputText(field.label.c_str(), buffer.data(), buffer.size()))
            changed = std::string(buffer.data());
        break;
    }
    case PropertyKind::Enumeration:
    {
        auto value = std::get<EnumerationValue>(*model.value).value;
        const auto preview = value.empty() ? "Choose…" : value.c_str();
        if (ImGui::BeginCombo(field.label.c_str(), preview))
        {
            for (const auto& option : field.enumValues)
                if (ImGui::Selectable(option.c_str(), option == value)) changed = EnumerationValue{option};
            ImGui::EndCombo();
        }
        break;
    }
    case PropertyKind::Vector:
    {
        const auto original = std::get<VectorValue>(*model.value);
        std::array<float, 4> value{};
        for (std::uint8_t index = 0; index < original.dimensions; ++index)
            value[index] = static_cast<float>(original.values[index]);
        const bool vectorChanged = original.dimensions == 2 ? ImGui::DragFloat2(field.label.c_str(), value.data(), 0.1f)
            : original.dimensions == 3 ? ImGui::DragFloat3(field.label.c_str(), value.data(), 0.1f)
            : ImGui::DragFloat4(field.label.c_str(), value.data(), 0.1f);
        if (vectorChanged)
        {
            auto updated = original;
            for (std::uint8_t index = 0; index < original.dimensions; ++index)
                updated.values[index] = value[index];
            changed = updated;
        }
        break;
    }
    case PropertyKind::AssetReference:
    {
        const auto& value = std::get<AssetReferenceValue>(*model.value).value;
        const auto buttonLabel = model.actualDisplayPath.empty() ? "Choose asset…" : model.actualDisplayPath.c_str();
        if (ImGui::Button(buttonLabel)) ImGui::OpenPopup("Choose asset");
        if (value)
        {
            ImGui::SameLine();
            if (ImGui::SmallButton("Edit"))
                intents.Submit("editor.asset.open", {{"projectId", value->project.ToString()},
                    {"assetId", value->asset.ToString()}, {"type", field.expectedAssetType}});
        }
        if (ImGui::BeginDragDropTarget())
        {
            if (const auto* payload = ImGui::AcceptDragDropPayload("AETHER_ASSET_REF"))
            {
                try
                {
                    const auto data = Json::parse(static_cast<const char*>(payload->Data));
                    if (data.value("type", std::string{}) == field.expectedAssetType)
                    {
                        auto project = ProjectAssets::ProjectId::Parse(data.value("projectId", std::string{}));
                        auto asset = ProjectAssets::AssetId::Parse(data.value("assetId", std::string{}));
                        if (project && asset) changed = AssetReferenceValue{ProjectAssets::ProjectAssetRef{*project, *asset}};
                    }
                }
                catch (...) {}
            }
            ImGui::EndDragDropTarget();
        }
        if (ImGui::BeginPopup("Choose asset"))
        {
            if (field.nullable && ImGui::Selectable("None", !value)) changed = AssetReferenceValue{};
            for (const auto& record : catalog.Records())
            {
                if (record.type != field.expectedAssetType) continue;
                const bool selected = value && value->asset == record.id;
                const auto option = record.displayPath + "  [" + record.type + "]";
                if (ImGui::Selectable(option.c_str(), selected))
                    changed = AssetReferenceValue{ProjectAssets::ProjectAssetRef{catalog.Project(), record.id}};
            }
            ImGui::EndPopup();
        }
        ImGui::SameLine();
        ImGui::TextDisabled("%s", field.expectedAssetType.c_str());
        break;
    }
    case PropertyKind::FeaturePayload:
        ImGui::Text("%s", model.label.c_str());
        ImGui::SameLine();
        ImGui::TextDisabled("Feature data");
        break;
    }
    ImGui::PopID();
    return changed;
}

void DrawInspector(const World& editorWorld, UiIntentSink& intents)
{
    if (!ImGui::Begin("Inspector")) { ImGui::End(); return; }
    const auto active = FindActiveWorld(editorWorld);
    const auto* services = FindServices(editorWorld);
    if (!active || !active->selectedEntity || !services || !services->runtime ||
        !services->componentSchemas || !services->catalog)
    {
        ImGui::TextUnformatted("Select an entity in an open World to inspect its properties.");
        ImGui::End();
        return;
    }
    const auto model = BuildInspectorModel(*active->world,
        {active->worldDocument->instanceId, *active->selectedEntity}, active->worldDocument->instanceId,
        *services->runtime, *services->componentSchemas, *services->catalog);
    if (!model.validTarget) ImGui::TextWrapped("%s", model.error.c_str());
    for (const auto& component : model.components)
    {
        ImGui::PushID(component.componentType.c_str());
        if (ImGui::CollapsingHeader(component.componentType.c_str(), ImGuiTreeNodeFlags_DefaultOpen))
        {
            const auto* schema = services->componentSchemas->Find(component.componentType);
            for (const auto& fieldModel : component.fields)
            {
                const auto* field = schema ? schema->FindField(fieldModel.fieldId) : nullptr;
                if (!field) continue;
                if (const auto changed = DrawProperty(*field, fieldModel, *services->catalog, intents))
                {
                    Json payload{{"entityId", active->selectedEntity->ToString()},
                        {"componentType", component.componentType}, {"fieldId", field->id},
                        {"value", field->EncodeValue(*changed)}};
                    SubmitEdit(intents, *active->document, *active->worldDocument,
                               "aether.component.set-field", std::move(payload));
                }
                if (!fieldModel.error.empty())
                    ImGui::TextColored({1.0f, 0.35f, 0.25f, 1.0f}, "%s", fieldModel.error.c_str());
            }
            ImGui::SameLine();
            if (schema && schema->removeComponent && ImGui::SmallButton("Remove"))
                SubmitEdit(intents, *active->document, *active->worldDocument,
                           "aether.component.remove", {{"entityId", active->selectedEntity->ToString()},
                               {"componentType", component.componentType}});
        }
        ImGui::PopID();
    }
    if (ImGui::Button("Add Component")) ImGui::OpenPopup("Add component");
    if (ImGui::BeginPopup("Add component"))
    {
        const auto selectedEntity = FindPersistentEntity(*active->world, *active->selectedEntity);
        for (const auto& [type, schema] : services->componentSchemas->Entries())
        {
            if (selectedEntity == entt::null || !schema.addComponent || schema.hasComponent(*active->world, selectedEntity))
                continue;
            if (ImGui::Selectable(type.c_str()))
                SubmitEdit(intents, *active->document, *active->worldDocument,
                           "aether.component.add", {{"entityId", active->selectedEntity->ToString()},
                               {"componentType", type}});
        }
        ImGui::EndPopup();
    }
    ImGui::End();
}

void DrawAssetBrowser(const World& world, UiIntentSink& intents)
{
    if (!ImGui::Begin("Assets")) { ImGui::End(); return; }
    const auto* services = FindServices(world);
    const auto* project = FindProject(world);
    if (!services || !services->catalog)
    {
        ImGui::TextUnformatted("Open a project to browse its assets.");
        ImGui::End();
        return;
    }
    for (const auto entity : world.Select<EditorStatusComponent>())
    {
        const auto& status = world.GetComponent<EditorStatusComponent>(entity);
        if (!status.message.empty())
        {
            if (status.isError) ImGui::TextColored({1.0f, 0.35f, 0.25f, 1.0f}, "%s", status.message.c_str());
            else ImGui::TextWrapped("%s", status.message.c_str());
        }
    }
    static char path[1024]{};
    static char importer[256]{};
    static char renamePath[1024]{};
    static std::string renameAssetId;
    static bool openRenamePopup = false;
    ImGui::InputText("Source file", path, sizeof(path));
    ImGui::InputText("Importer", importer, sizeof(importer));
    const bool writable = project && project->open && project->writable;
    if (!writable) ImGui::BeginDisabled();
    if (ImGui::Button("Import"))
        intents.Submit("editor.asset.import", {{"path", path}, {"importerId", importer}});
    if (!writable) ImGui::EndDisabled();
    ImGui::Separator();
    for (const auto& asset : services->catalog->Records())
    {
        ImGui::PushID(asset.id.ToString().c_str());
        if (ImGui::Selectable(asset.displayPath.c_str()))
            intents.Submit("editor.asset.open", {{"projectId", services->catalog->Project().ToString()},
                {"assetId", asset.id.ToString()}, {"type", asset.type}});
        if (ImGui::BeginDragDropSource())
        {
            const auto payload = Json{{"projectId", services->catalog->Project().ToString()},
                {"assetId", asset.id.ToString()}, {"type", asset.type}}.dump();
            ImGui::SetDragDropPayload("AETHER_ASSET_REF", payload.c_str(), payload.size() + 1);
            ImGui::TextUnformatted(asset.displayPath.c_str());
            ImGui::EndDragDropSource();
        }
        ImGui::SameLine();
        ImGui::TextDisabled("%s", asset.type.c_str());
        if (ImGui::BeginPopupContextItem("Asset actions"))
        {
            if (ImGui::MenuItem("Open"))
                intents.Submit("editor.asset.open", {{"projectId", services->catalog->Project().ToString()},
                    {"assetId", asset.id.ToString()}, {"type", asset.type}});
            if (!writable) ImGui::BeginDisabled();
            if (ImGui::MenuItem("Rename"))
            {
                renameAssetId = asset.id.ToString();
                std::snprintf(renamePath, sizeof(renamePath), "%s", asset.displayPath.c_str());
                openRenamePopup = true;
            }
            if (!writable) ImGui::EndDisabled();
            ImGui::EndPopup();
        }
        ImGui::PopID();
    }
    if (openRenamePopup)
    {
        ImGui::OpenPopup("Rename asset");
        openRenamePopup = false;
    }
    if (ImGui::BeginPopupModal("Rename asset", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::InputText("Display path", renamePath, sizeof(renamePath));
        if (ImGui::Button("Rename") && project && project->open && project->writable)
        {
            intents.Submit("editor.asset.rename", {{"projectId", services->catalog->Project().ToString()},
                {"sessionGeneration", project->sessionGeneration},
                {"catalogGeneration", services->catalog->Generation()}, {"assetId", renameAssetId},
                {"displayPath", renamePath}});
            renameAssetId.clear();
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel"))
        {
            renameAssetId.clear();
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
    for (const auto entity : world.Select<AssetImportResultComponent>())
    {
        const auto& result = world.GetComponent<AssetImportResultComponent>(entity);
        if (result.state == AssetImportTaskState::Running || result.state == AssetImportTaskState::Queued)
            ImGui::Text("Import in progress…");
        else if (result.state == AssetImportTaskState::Rejected)
            ImGui::TextWrapped("Import failed: %s", result.error.c_str());
    }
    ImGui::End();
}

void DrawToolbar(const World& world, UiIntentSink& intents)
{
    if (!ImGui::Begin("Toolbar")) { ImGui::End(); return; }
    const auto active = FindActiveWorld(world);
    if (active)
    {
        ImGui::TextUnformatted(active->document->dirty ? "Unsaved changes" : "Saved");
        if (ImGui::Button("Save"))
            SubmitDocumentIntent(intents, "editor.document.save", active->document->id);
        ImGui::SameLine();
        if (ImGui::Button("Undo")) SubmitDocumentIntent(intents, "editor.history.undo", active->document->id);
        ImGui::SameLine();
        if (ImGui::Button("Redo")) SubmitDocumentIntent(intents, "editor.history.redo", active->document->id);
    }
    const PlaySessionComponent* play = nullptr;
    for (const auto entity : world.Select<PlaySessionComponent>()) { play = &world.GetComponent<PlaySessionComponent>(entity); break; }
    if (!play || play->state == PlayState::Stopped || play->state == PlayState::Failed)
    {
        if (ImGui::Button("Play")) intents.Submit("editor.play.start", Json::object());
    }
    else
    {
        if (play->state == PlayState::Paused)
        {
            if (ImGui::Button("Resume")) intents.Submit("editor.play.resume", Json::object());
            ImGui::SameLine();
            if (ImGui::Button("Step")) intents.Submit("editor.play.step", Json::object());
        }
        else if (ImGui::Button("Pause")) intents.Submit("editor.play.pause", Json::object());
        ImGui::SameLine();
        if (ImGui::Button("Stop")) intents.Submit("editor.play.stop", Json::object());
    }
    if (play && !play->error.empty()) ImGui::TextWrapped("%s", play->error.c_str());
    for (const auto entity : world.Select<EditCommandResultComponent>())
    {
        const auto& result = world.GetComponent<EditCommandResultComponent>(entity);
        if (result.state == EditRequestState::Rejected && !result.error.empty())
            ImGui::TextColored({1.0f, 0.35f, 0.25f, 1.0f}, "Edit failed: %s", result.error.c_str());
    }
    ImGui::End();
}

void DrawViewports(const World& world, UiIntentSink& intents)
{
    for (const auto entity : world.Select<ViewportPanelComponent>())
    {
        const auto& panel = world.GetComponent<ViewportPanelComponent>(entity);
        const auto title = panel.model.request.viewId.empty() ? "Viewport" : panel.model.request.viewId;
        if (!ImGui::Begin(title.c_str())) { ImGui::End(); continue; }
        const auto available = ImGui::GetContentRegionAvail();
        if (available.x > 0.0f && available.y > 0.0f &&
            (panel.model.request.logicalWidth != available.x || panel.model.request.logicalHeight != available.y))
            intents.Submit("editor.viewport.resize", {{"viewId", panel.model.request.viewId},
                {"documentId", panel.model.request.documentId.ToString()}, {"width", available.x}, {"height", available.y}});
        const bool focused = ImGui::IsWindowFocused();
        if (focused != panel.focused)
            intents.Submit("editor.viewport.focus", {{"viewId", panel.model.request.viewId}, {"focused", focused}});
        if (panel.model.status == ViewportStatus::MissingProvider)
            ImGui::TextWrapped("No preview is available for this view.");
        else if (panel.model.status == ViewportStatus::ProviderError)
            ImGui::TextWrapped("Preview error: %s", panel.model.error.c_str());
        else if (panel.model.status == ViewportStatus::Suspended)
            ImGui::TextUnformatted("Preview is paused.");
        else
            ImGui::TextUnformatted("Preview ready");
        if (ImGui::IsWindowHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
        {
            const auto position = ImGui::GetMousePos();
            const auto origin = ImGui::GetWindowPos();
            const auto size = ImGui::GetWindowSize();
            if (size.x > 0 && size.y > 0)
                intents.Submit("editor.viewport.pointer", {{"viewId", panel.model.request.viewId},
                    {"generation", panel.model.request.generation},
                    {"x", std::clamp((position.x - origin.x) / size.x, 0.0f, 1.0f)},
                    {"y", std::clamp((position.y - origin.y) / size.y, 0.0f, 1.0f)}});
        }
        ImGui::End();
    }
}

std::pair<std::string_view, UiSystemRegistry::DrawCallback> PanelEntry(CorePanel panel)
{
    switch (panel)
    {
    case CorePanel::DocumentTabs: return {"core.documents", DrawDocumentTabs};
    case CorePanel::Hierarchy: return {"core.hierarchy", DrawHierarchy};
    case CorePanel::Inspector: return {"core.inspector", DrawInspector};
    case CorePanel::AssetBrowser: return {"core.assets", DrawAssetBrowser};
    case CorePanel::Toolbar: return {"core.toolbar", DrawToolbar};
    case CorePanel::Viewport: return {"core.viewport", DrawViewports};
    }
    return {};
}
} // namespace

bool RegisterCorePanels(UiSystemRegistry& systems, std::span<const CorePanel> panels)
{
    static constexpr std::array allPanels{CorePanel::DocumentTabs, CorePanel::Hierarchy,
        CorePanel::Inspector, CorePanel::AssetBrowser, CorePanel::Toolbar, CorePanel::Viewport};
    const auto selected = panels.empty() ? std::span<const CorePanel>(allPanels) : panels;
    std::vector<std::string> registered;
    for (const auto panel : selected)
    {
        const auto [id, draw] = PanelEntry(panel);
        if (id.empty() || !systems.Register(std::string(id), draw))
        {
            for (const auto& previous : registered) systems.Unregister(previous);
            return false;
        }
        registered.emplace_back(id);
    }
    return true;
}
} // namespace Aether::EditorImGui
