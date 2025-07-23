#pragma once
#include <vector>
#include <Render/RenderApi.h>
#include <Core/Core.h>
#include "TaskBuilder.h"
#include "Resource.h"
#include "Realize.h"
#include "TaskBase.h"
#include <variant>
#include "TransientResourcePool.h"
#include "RenderTask.h"
#include "Task.h"
#include "TaskArray.h"
#include "RealizeImpl.h"
namespace Aether::TaskGraph
{

class TaskGraph
{
    friend class RenderTaskBuilder;

public:
    template <typename TaskData>
    TaskData AddRenderTask(const RenderPassDesc& renderPassDesc, std::function<void(RenderTaskBuilder&, TaskData&)> setup,
                           std::function<void(TaskData&, DeviceCommandBufferView& commandBuffer)> execute)
    {
        m_RenderTasks.emplace_back(CreateScope<RenderTask<TaskData>>(renderPassDesc, setup, execute));
        auto* task = (RenderTask<TaskData>*)m_RenderTasks.back().get();
        RenderTaskBuilder builder(this, task);
        task->Setup(builder);
        for(size_t i = 0; i < renderPassDesc.colorAttachmentCount; ++i)
        {
            auto& colorAttachment = renderPassDesc.colorAttachment[i];
            assert(colorAttachment.texture && "color attachment texture is null");
            builder.Write(colorAttachment.texture);
        }
        if (renderPassDesc.depthAttachment)
        {
            auto& depthAttachment = renderPassDesc.depthAttachment.value();
            assert(depthAttachment.texture && "depth attachment texture is null");
            builder.Write(depthAttachment.texture);
        }
        return task->GetData();
    }
    void Compile()
    {
        CalculateTimeline();
        MergeRenderPass();
    }
    template <typename ActualType, typename DescType>
    Resource<ActualType, DescType>* AddRetainedResource(const std::string& tag, Scope<ActualType> actual, const DescType& desc)
    {
        auto r = CreateScope<Resource<ActualType, DescType>>(tag, nullptr, desc);
        r->SetActual(std::move(actual));
        m_Resources.emplace_back(std::move(r));
        return static_cast<Resource<ActualType, DescType>*>(m_Resources.back().get());
    }
    template <typename ActualType, typename DescType>
    Resource<ActualType, DescType>* AddRetainedResourceBorrow(const std::string& tag, Borrow<ActualType> actual, const DescType& desc)
    {
        auto r = CreateScope<Resource<ActualType, DescType>>(tag, nullptr, desc);
        r->SetActualBorrow(actual);
        m_Resources.emplace_back(std::move(r));
        return static_cast<Resource<ActualType, DescType>*>(m_Resources.back().get());
    }
    bool CheckGraph()
    {
        // TODO: check if cycle exists
        return true;
    }
    void Execute()
    {
        for (auto& timeline : m_Timelines)
        {
            for (auto& resource : timeline.realizedResources)
            {
                RealizeResource(*resource);
            }
            ExecuteTask executeTask{m_CommandBuffer,m_TransientResourcePool};
            std::visit(executeTask, timeline.task);
            for (auto& resource : timeline.derealizedResources)
            {
                DerealizeResource(*resource);
            }
        }
    }
    void SetCommandBuffer(DeviceCommandBufferView commandBuffer)
    {
        m_CommandBuffer = commandBuffer;
    }
private:
    /**
     * @brief Merge consecutive render tasks that use the same render pass into a render task array
    */
    void MergeRenderPass();
    void CalculateTimeline();
private:
    
    void RealizeResource(ResourceBase& resource)
    {
        switch (resource.type)
        {
        default:
            resource.Realize();
        }
    }
    void DerealizeResource(ResourceBase& resource)
    {
        switch (resource.type)
        {
        default:
            resource.Derealize();
        }
    }
    
    struct Timeline
    {
        Task task;
        std::vector<Borrow<ResourceBase>> realizedResources;   // should realize before task
        std::vector<Borrow<ResourceBase>> derealizedResources; // should derealize after task
    };
    struct ExecuteTask
    {
        DeviceCommandBufferView& commandBuffer;
        TransientResourcePool& m_TransientResourcePool;
        void operator()(const std::monostate&)
        {
            return;
        }
        void operator()(const Borrow<RenderTaskBase>& task)
        {
            
            // get or create render pass
            auto renderPass=m_TransientResourcePool.PopRenderPass(task->GetRenderPassDesc());
            if(!renderPass)
            {
                renderPass=CreateRenderPass(task->GetRenderPassDesc());
                assert(renderPass);
            }
            // get or create frame buffer
            auto frameBufferDesc=RenderPassDescToFrameBufferDesc(task->GetRenderPassDesc());
            auto frameBuffer=m_TransientResourcePool.PopFrameBuffer(frameBufferDesc);
            if(!frameBuffer)
            {
                frameBuffer=CreateFrameBuffer(frameBufferDesc,*renderPass);
                assert(frameBuffer);
            }
            // render pass
            commandBuffer.GetVk().BeginRenderPass(renderPass->GetVk(),
            frameBuffer->GetVk(), 
            task->GetRenderPassDesc().clearColor);

            task->Execute(commandBuffer);
            commandBuffer.GetVk().EndRenderPass();
            // push render pass and frame buffer to transient resource pool
            m_TransientResourcePool.PushRenderPass(std::move(renderPass),task->GetRenderPassDesc());
            m_TransientResourcePool.PushFrameBuffer(std::move(frameBuffer),frameBufferDesc);
        }
        void operator()(const Borrow<RenderTaskArray>& task)
        {
            // get or create render pass
            auto renderPass=m_TransientResourcePool.PopRenderPass(task->renderPassDesc);
            if(!renderPass)
            {
                renderPass=CreateRenderPass(task->renderPassDesc);
                assert(renderPass);
            }
            // get or create frame buffer
            auto frameBufferDesc=RenderPassDescToFrameBufferDesc(task->renderPassDesc);
            auto frameBuffer=m_TransientResourcePool.PopFrameBuffer(frameBufferDesc);
            if(!frameBuffer)
            {
                frameBuffer=CreateFrameBuffer(frameBufferDesc,*renderPass);
                assert(frameBuffer);
            }
            // render pass
            commandBuffer.GetVk().BeginRenderPass(renderPass->GetVk(),
            frameBuffer->GetVk(), 
            task->renderPassDesc.clearColor);
            for(auto& t:task->tasks)
            {
                t->Execute(commandBuffer);
            }
            commandBuffer.GetVk().EndRenderPass();
            // push render pass and frame buffer to transient resource pool
            m_TransientResourcePool.PushRenderPass(std::move(renderPass),task->renderPassDesc);
            m_TransientResourcePool.PushFrameBuffer(std::move(frameBuffer),frameBufferDesc);
        }
        FrameBufferDesc RenderPassDescToFrameBufferDesc(const RenderPassDesc& desc)
        {
            FrameBufferDesc frameBufferDesc;
            frameBufferDesc.colorAttachmentCount=desc.colorAttachmentCount;
            uint32_t width=0;
            uint32_t height=0;
            if(desc.colorAttachmentCount)
            {
                width=desc.colorAttachment[0].texture->GetDesc().width;
                height=desc.colorAttachment[0].texture->GetDesc().height;
            }
            else if(desc.depthAttachment)
            {
                width=desc.depthAttachment->texture->GetDesc().width;
                height=desc.depthAttachment->texture->GetDesc().height;
            }
            else
            {
               assert(false&&"no attachment in render pass");
            }
            frameBufferDesc.width=width;
            frameBufferDesc.height=height;
            for(size_t i=0;i<desc.colorAttachmentCount;++i)
            {
                auto& texture=desc.colorAttachment[i].texture;
                frameBufferDesc.colorAttachments[i]=texture;
            }
            if(desc.depthAttachment)
            {
                frameBufferDesc.depthAttachment=desc.depthAttachment->texture;
            }
            return frameBufferDesc;
        }
        Scope<DeviceFrameBuffer> CreateFrameBuffer(const FrameBufferDesc& desc,DeviceRenderPass& renderPass)
        {
            DeviceFrameBufferDesc frameBufferDesc;
            frameBufferDesc.colorAttachmentCount=desc.colorAttachmentCount;
            for(size_t i=0;i<desc.colorAttachmentCount;++i)
            {
                auto* texture=desc.colorAttachments[i];
                DeviceTexture* actual;
                if(texture->Transient())
                {
                    actual = texture->GetActual().get();
                }
                else
                {
                    actual = texture->GetActualBorrow().Get();
                }
                frameBufferDesc.colorAttachments[i]=&actual->GetOrCreateDefaultImageView();
            }
            if(desc.depthAttachment)
            {
                DeviceTexture* actual;
                if(desc.depthAttachment->Transient())
                {
                    actual = desc.depthAttachment->GetActual().get();
                }
                else
                {
                    actual = desc.depthAttachment->GetActualBorrow().Get();
                }
                frameBufferDesc.depthAttachment= &actual->GetOrCreateDefaultImageView();
            }
            frameBufferDesc.width=desc.width;
            frameBufferDesc.height=desc.height;
            auto deviceFrameBuffer=DeviceFrameBuffer::Create(renderPass,frameBufferDesc);
            if(deviceFrameBuffer.Empty())
            {
                return nullptr;
            }
            return CreateScope<DeviceFrameBuffer>(std::move(deviceFrameBuffer));
        }
        Scope<DeviceRenderPass> CreateRenderPass(const RenderPassDesc& desc)
        {
            DeviceRenderPassDesc renderPassDesc;
            renderPassDesc.colorAttachmentCount=desc.colorAttachmentCount;
            for(size_t i=0;i<desc.colorAttachmentCount;++i)
            {
                renderPassDesc.colorAttachments[i].load=desc.colorAttachment[i].loadOp;
                renderPassDesc.colorAttachments[i].store=desc.colorAttachment[i].storeOp;
                renderPassDesc.colorAttachments[i].format=desc.colorAttachment[i].texture->GetDesc().pixelFormat;
            }
            if(desc.depthAttachment)
            {
                DeviceAttachmentDesc depthAttachmentDesc;
                depthAttachmentDesc.load=desc.depthAttachment->loadOp;
                depthAttachmentDesc.store=desc.depthAttachment->storeOp;
                depthAttachmentDesc.format=desc.depthAttachment->texture->GetDesc().pixelFormat;
                renderPassDesc.depthAttachment=depthAttachmentDesc;
            }
            auto renderPass=DeviceRenderPass::Create(renderPassDesc);
            if(renderPass.Empty())
            {
                return nullptr;
            }
            return CreateScope<DeviceRenderPass>(std::move(renderPass));
        }
    };
    std::vector<Scope<ResourceBase>> m_Resources;
    std::vector<Scope<RenderTaskBase>> m_RenderTasks;
    std::vector<Timeline> m_Timelines; // create on compile, execute every frame
private:                               // render resource
    DeviceCommandBufferView m_CommandBuffer;
    TransientResourcePool m_TransientResourcePool;
};
template <typename ResourceType, typename Desc>
    requires std::derived_from<ResourceType, ResourceBase>
ResourceType* RenderTaskBuilder::Create(const std::string& tag, const Desc& desc)
{
    m_TaskGraph->m_Resources.emplace_back(CreateScope<ResourceType>(tag, m_Task, desc));

    auto* resource = m_TaskGraph->m_Resources.back().get();
    m_Task->creates.push_back(resource);
    return static_cast<ResourceType*>(resource);
}
template <typename ResourceType>
    requires std::derived_from<ResourceType, ResourceBase>
inline ResourceType* RenderTaskBuilder::Read(ResourceType* _resource)
{
    ResourceBase* resource = (ResourceBase*)_resource;

    resource->readers.push_back(m_Task);
    m_Task->reads.push_back(resource);
    
    return _resource;
}
template <typename ResourceType>
    requires std::derived_from<ResourceType, ResourceBase>
inline ResourceType* RenderTaskBuilder::Write(ResourceType* _resource)
{
    ResourceBase* resource = (ResourceBase*)_resource;

    resource->writers.push_back(m_Task);
    m_Task->writes.push_back(resource);
    return _resource;
}

} // namespace Aether::TaskGraph