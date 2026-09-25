#include <EditorFramework/Viewport.h>

#include <cassert>

int main()
{
    using namespace Aether;
    using namespace Aether::EditorFramework;
    using namespace Aether::GameFeatures;

    ViewportProviderRegistry providers;
    const auto pickedEntity = PersistentEntityId::Create();
    ViewportProvider fake;
    fake.id = "sample.fake-provider";
    fake.ownerFeature = "sample.runtime";
    fake.update = [](const ViewportRequest& request) -> std::expected<ViewportOutput, std::string> {
        auto content = std::make_shared<std::vector<std::byte>>();
        content->push_back(static_cast<std::byte>(request.purpose));
        return ViewportOutput{request.generation, {}, std::move(content), {}};
    };
    fake.pick = [pickedEntity](const ViewportPickRequest& request)
        -> std::expected<std::optional<PersistentEntityId>, std::string> {
        assert(request.normalizedX >= 0.0 && request.normalizedX <= 1.0);
        return std::optional<PersistentEntityId>{pickedEntity};
    };
    assert(providers.Register(fake));
    assert(!providers.Register(fake));

    ViewportRouter router;
    const auto document = DocumentId::Create();
    const auto authoringWorld = WorldInstanceId::Create();
    const auto playWorld = WorldInstanceId::Create();
    const auto previewWorld = WorldInstanceId::Create();
    assert(router.Upsert({"view.main", document, authoringWorld, ViewportPurpose::View, fake.id, 100.0, 50.0, 2.0, 1}));
    assert(router.Upsert({"view.play", document, playWorld, ViewportPurpose::Play, fake.id, 100.0, 50.0, 1.0, 1}));
    assert(router.Upsert({"view.preview", document, previewWorld, ViewportPurpose::AssetPreview, fake.id, 64.0, 64.0, 1.0, 1}));

    auto view = router.Update("view.main", providers);
    auto play = router.Update("view.play", providers);
    auto preview = router.Update("view.preview", providers);
    assert(view && play && preview);
    assert(view->status == ViewportStatus::Ready && play->status == ViewportStatus::Ready
        && preview->status == ViewportStatus::Ready);
    assert(view->output.ownedPayload != play->output.ownedPayload);
    assert(view->request.worldInstanceId != play->request.worldInstanceId
        && play->request.worldInstanceId != preview->request.worldInstanceId);

    assert(!router.RoutePointer("view.main", {10.0, 10.0, 1, 0}));
    assert(router.Focus("view.main"));
    auto routed = router.RoutePointer("view.main", {100.0, 50.0, 3, 4});
    assert(routed && routed->normalizedX == 0.5 && routed->normalizedY == 0.5);
    router.SetUiCapture(true, false);
    assert(!router.RoutePointer("view.main", {10.0, 10.0, 1, 0}));
    router.SetUiCapture(false, false);
    assert(!router.RoutePointer("view.main", {10.0, 10.0, 1, 0}));
    assert(router.Focus("view.main"));
    auto pick = router.RequestPick("view.main", 100.0, 50.0, providers);
    assert(pick && pick->normalizedX == 0.5 && pick->normalizedY == 0.5);
    auto picked = router.ExecutePick(*pick, providers);
    assert(picked && picked->entity == pickedEntity);

    auto resized = router.Find("view.main")->request;
    resized.logicalWidth = 150.0;
    assert(router.Upsert(resized));
    assert(router.Find("view.main")->request.generation == 2);
    assert(!router.AcceptPickResult(*picked));
    auto updated = router.Update("view.main", providers);
    assert(updated && updated->output.generation == 2);

    auto noProvider = router.Upsert({"view.empty", document, authoringWorld, ViewportPurpose::View,
        "sample.no-provider", 30.0, 20.0, 1.0, 1});
    assert(noProvider);
    auto missing = router.Update("view.empty", providers);
    assert(missing && missing->status == ViewportStatus::MissingProvider);
    assert(router.Upsert({"view.suspended", document, authoringWorld, ViewportPurpose::View,
        fake.id, 0.0, 0.0, 1.0, 1}));
    auto suspended = router.Update("view.suspended", providers);
    assert(suspended && suspended->status == ViewportStatus::Suspended);
    assert(router.Suspend("view.main", "Loading viewport surface"));
    const auto loading = router.Find("view.main");
    assert(loading && loading->status == ViewportStatus::Suspended
        && loading->error == "Loading viewport surface" && !loading->output.surface.IsValid()
        && !loading->output.ownedPayload);
    router.Remove("view.main");
    assert(!router.FocusedView());
}
