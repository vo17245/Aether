#include "doctest/doctest.h"
#include <Render/RenderGraph/Handle.h>
#include <Render/RenderGraph/RenderGraph.h>
#include <Render/RHI/Backend/Vulkan/VulkanUtils.h>
#include <array>
#include <memory>

using namespace Aether;
namespace RG = Aether::RenderGraph;

TEST_CASE("RenderGraph handles recycle without reviving stale versions")
{
    RG::HandleAllocator allocator;
    auto first = allocator.Allocate();
    CHECK(allocator.IsActive(first));
    CHECK(allocator.Free(first));
    CHECK_FALSE(allocator.IsActive(first));
    CHECK_FALSE(allocator.Free(first));
    auto next = allocator.Allocate();
    CHECK(next.id == first.id);
    CHECK(next.version != first.version);
    CHECK_FALSE(allocator.IsActive(first));
    CHECK(allocator.Free(next));
    for (int i = 0; i < 70000; ++i)
    {
        auto id = allocator.Allocate();
        CHECK(id.IsValid());
        CHECK(allocator.Free(id));
    }
    CHECK(allocator.RemainingCapacity() == RG::Handle::InvalidId);
    std::vector<RG::Handle> all;
    for (unsigned i=0; i<RG::Handle::InvalidId; ++i) all.push_back(allocator.Allocate());
    CHECK(allocator.RemainingCapacity() == 0);
    CHECK_FALSE(allocator.TryAllocate().has_value());
    for (auto id:all) CHECK(allocator.Free(id));
}

TEST_CASE("Arena imports index and uniform buffers without taking ownership")
{
    RG::ResourceArena arena;
    RG::ResourceLruPool pool(&arena);
    rhi::VertexBuffer vertices;
    rhi::IndexBuffer indices;
    rhi::UniformBuffer frame[2];
    auto before=arena.RemainingIdCapacity();
    auto vertexId=arena.Import(&vertices);
    auto indexId=arena.Import(&indices);
    auto frame0=arena.Import(&frame[0]);
    auto frame1=arena.Import(&frame[1]);
    auto owned=std::make_unique<rhi::IndexBuffer>();
    auto* ownedAddress=owned.get();
    auto ownedId=arena.AddIndexBuffer(std::move(owned));
    CHECK(arena.GetResource(ownedId)==ownedAddress);
    CHECK(arena.GetResource(vertexId)==&vertices);
    CHECK(arena.GetResource(indexId)==&indices);
    CHECK(arena.GetResource(frame0)==&frame[0]);
    CHECK(arena.RemainingIdCapacity()==before-5);
    {
        RG::RenderGraph graph(&arena,&pool);
        auto vb=graph.Import<rhi::VertexBuffer>("test.vb",RG::VertexBufferDesc{28},
                 std::span<const RG::ResourceId<rhi::VertexBuffer>>(&vertexId,1));
        auto ib=graph.Import<rhi::IndexBuffer>("test.ib",RG::IndexBufferDesc{24},
                 std::span<const RG::ResourceId<rhi::IndexBuffer>>(&indexId,1));
        std::array frameIds{frame0,frame1};
        auto ubo=graph.Import<rhi::UniformBuffer>("test.ubo",RG::UniformBufferDesc{128},
                 std::span<const RG::ResourceId<rhi::UniformBuffer>>(frameIds));
        graph.AddRenderTask<int>("test.read",
            [&](RG::RenderTaskBuilder& builder,int&) {
                builder.Read(vb);builder.Read(ib);builder.Read(ubo);
            },[](rhi::CommandList&,RG::ResourceAccessor&,int&){});
        CHECK(graph.GetVirtualResourceById(vb)->readers.size()==1);
        CHECK(graph.GetVirtualResourceById(ib)->readers.size()==1);
        CHECK(graph.GetVirtualResourceById(ubo)->readers.size()==1);
        graph.SetCurrentFrame(0);
        CHECK(graph.GetResourceAccessor().GetResource(vb)==&vertices);
        CHECK(graph.GetResourceAccessor().GetResource(ib)==&indices);
        CHECK(graph.GetResourceAccessor().GetResource(ubo)==&frame[0]);
        graph.SetCurrentFrame(1);
        CHECK(graph.GetResourceAccessor().GetResource(ubo)==&frame[1]);
    }
    arena.Destroy(indexId);
    CHECK_FALSE(arena.IsValid(indexId));
    CHECK(arena.GetResource(indexId)==nullptr);
    arena.Destroy(indexId);
    CHECK(arena.GetResource(ownedId)==ownedAddress);
    arena.Destroy(ownedId);
    arena.Destroy(vertexId);
    arena.Destroy(frame0);
    arena.Destroy(frame1);
    CHECK(arena.RemainingIdCapacity()==before);

    rhi::Texture2D texture;
    rhi::TextureView view;
    auto textureId=arena.Import(&texture);
    auto viewId=arena.Import(&view);
    arena.AddDependency(viewId,textureId);
    arena.Destroy(viewId);
    CHECK_FALSE(arena.IsValid(viewId));
    auto reusedView=arena.Import(&view);
    CHECK(reusedView.handle.id==viewId.handle.id);
    CHECK(arena.IsValid(reusedView));
    arena.Destroy(textureId);
    CHECK(arena.IsValid(reusedView));
    arena.Destroy(reusedView);
    CHECK(arena.RemainingIdCapacity()==before);
}

TEST_CASE("Swapchain format selection requires UNORM and SRGB_NONLINEAR together")
{
    const std::vector<VkSurfaceFormatKHR> supported={
        {VK_FORMAT_B8G8R8A8_SRGB,VK_COLOR_SPACE_SRGB_NONLINEAR_KHR},
        {VK_FORMAT_R8G8B8A8_UNORM,VK_COLOR_SPACE_DISPLAY_P3_NONLINEAR_EXT},
        {VK_FORMAT_B8G8R8A8_UNORM,VK_COLOR_SPACE_SRGB_NONLINEAR_KHR}};
    auto chosen=vk::chooseSwapSurfaceFormat(supported);
    CHECK(chosen.format==VK_FORMAT_B8G8R8A8_UNORM);
    CHECK(chosen.colorSpace==VK_COLOR_SPACE_SRGB_NONLINEAR_KHR);
    CHECK_THROWS_AS(vk::chooseSwapSurfaceFormat({supported[0],supported[1]}),std::runtime_error);
    CHECK_THROWS_AS(vk::chooseSwapSurfaceFormat({}),std::runtime_error);
}
