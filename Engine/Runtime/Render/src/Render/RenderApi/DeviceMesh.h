#pragma once
#include <string>
#include <variant>
#include "../Mesh/VkMesh.h"
#include <expected>
#include "../Config.h"
#include "Render/Config.h"
#include "Render/Vulkan/GlobalRenderContext.h"
namespace Aether
{
class DeviceMesh
{
public:
    DeviceMesh() = default;
    DeviceMesh(const DeviceMesh&) = delete;
    DeviceMesh(DeviceMesh&&) = default;
    DeviceMesh& operator=(const DeviceMesh&) = delete;
    DeviceMesh& operator=(DeviceMesh&&) = default;
    bool Empty() const
    {
        return m_Mesh.index() == 0;
    }
    operator bool()
    {
        return !Empty();
    }
    static DeviceMesh Create(const Mesh& mesh)
    {
        DeviceMesh res;
        switch (Render::Config::RenderApi)
        {
            case Render::Api::Vulkan:
            {
                auto vkMesh=VkMesh::Create(vk::GRC::GetGraphicsCommandPool(), mesh);
                if(!vkMesh)
                {
                    return res;
                }
                res.m_Mesh=std::move(vkMesh.value());
                return res;
            }
            break;
            default:
            {
                assert(false && "Not implemented");
                return res;
            }
        
        }
    }
    static DeviceMesh* CreateRaw(const Mesh& mesh)
    {
        DeviceMesh* res=new DeviceMesh();
        switch (Render::Config::RenderApi)
        {
            case Render::Api::Vulkan:
            {
                auto vkMesh=VkMesh::Create(vk::GRC::GetGraphicsCommandPool(), mesh);
                if(!vkMesh)
                {
                    delete res;
                    return nullptr;
                }
                res->m_Mesh=std::move(vkMesh.value());
                return res;
            }
            break;
            default:
            {
                assert(false && "Not implemented");
                delete res;
                return nullptr;
            }
        
        }
    }

    template<typename T>
    T& Get()
    {
        return std::get<T>(m_Mesh);
    }
    template<typename T>
    const T& Get() const
    {
        return std::get<T>(m_Mesh);
    }
    VkMesh& GetVk()
    {
        return Get<VkMesh>();
    }
    const VkMesh& GetVk() const
    {
        return Get<VkMesh>();
    } 
    void Update(const Mesh& mesh)
    {
        switch (Render::Config::RenderApi)
        {
            case Render::Api::Vulkan:
            {
                Get<VkMesh>().Update(mesh);
            }
            break;
            default:
            {
                assert(false && "Not implemented");
            }
        
        }
    }
private:
    std::variant<std::monostate, VkMesh> m_Mesh;
};
} // namespace Aether