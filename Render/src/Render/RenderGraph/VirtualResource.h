#pragma once
#include "Resource/Resource.h"
#include "Resource/ResourceTypeTraits.h"
#include <Core/Core.h>
#include "AccessId.h"
namespace Aether::RenderGraph
{
    enum class ResourceCode:uint32_t
    {
        Texture,
        ImageView,
        FrameBuffer,
        RenderPass,
        Buffer,
    };
    template<typename ResourceType>
    struct GetResourceCode;
    template<typename ResourceType,typename=void>
    struct HasGetResourceCodeImpl
    {
        static constexpr bool value=false;
    };
    template<typename ResourceType>
    struct HasGetResourceCodeImpl<ResourceType,std::void_t<decltype(GetResourceCode<ResourceType>::value)>>
    {
        static constexpr bool value=std::is_same_v<decltype(GetResourceCode<ResourceType>::value),ResourceCode>;
    };
    struct TaskBase;
    struct VirtualResourceBase
    {
        virtual ~VirtualResourceBase() = default;
        std::vector<Borrow<TaskBase>> readers;
        std::vector<Borrow<TaskBase>> writers;
        TaskBase* creator=nullptr;
        ResourceCode code;
    };
    template<typename ResourceType>
    requires IsResource<ResourceType>::value&&HasGetResourceCodeImpl<ResourceType>::value
    struct VirtualResource:public VirtualResourceBase
    {
        ResourceDescType<ResourceType>::Type desc;
        AccessId<ResourceType> id;
        VirtualResource()
        {
            code=GetResourceCode<ResourceType>::value;            
        }
    };

}
// specialize GetResourceCode for each resource type
#include "Resource/DeviceBuffer.h"
#include "Resource/DeviceTexture.h"
#include "Resource/DeviceImageView.h"
#include "Resource/DeviceFrameBuffer.h"
#include "Resource/DeviceRenderPass.h"
namespace Aether::RenderGraph
{
    template<>
    struct GetResourceCode<DeviceTexture>
    {
        static constexpr ResourceCode value=ResourceCode::Texture;
    };
    template<>
    struct GetResourceCode<DeviceImageView>
    {
        static constexpr ResourceCode value=ResourceCode::ImageView;
    };
    template<>
    struct GetResourceCode<DeviceFrameBuffer>
    {
        static constexpr ResourceCode value=ResourceCode::FrameBuffer;
    };
    template<>
    struct GetResourceCode<DeviceRenderPass>
    {
        static constexpr ResourceCode value=ResourceCode::RenderPass;
    };
    template<>
    struct GetResourceCode<DeviceBuffer>
    {
        static constexpr ResourceCode value=ResourceCode::Buffer;
    };
}