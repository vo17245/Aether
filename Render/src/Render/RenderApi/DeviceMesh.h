#pragma once
#include <string>
#include <variant>
#include "../Mesh/VkMesh.h"
#include <expected>
#include "../Config.h"
#include "Render/Vulkan/GlobalRenderContext.h"
namespace Aether
{
using DeviceMesh=std::variant<std::monostate,VkMesh>;
inline std::expected<DeviceMesh,std::string> CreateDeviceMesh(const Mesh& mesh)
{
    switch (Render::Config::RenderApi) {
        case Render::Api::Vulkan:
        {
            auto vkMesh=VkMesh::Create(vk::GRC::GetGraphicsCommandPool(),mesh);
            if(!vkMesh)
            {
                return std::unexpected<std::string>("Failed to create Vulkan Mesh");
            }
            return std::move(vkMesh.value());
        }
        default:
            return std::unexpected<std::string>("Unsupported Render API");
    }
}
}