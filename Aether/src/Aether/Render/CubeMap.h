#pragma once
#include "Aether/Core/Core.h"
#include "Aether/Render/OpenGLApi.h"
#include "Aether/Resource/Image.h"
#include <utility>
namespace Aether
{
    class CubeMap
    {
        public:
            CubeMap(const Image& px,const Image& nx,const Image& py,const Image& ny,const Image& pz,const Image& nz);
            CubeMap();
            ~CubeMap();
            void SetPositiveX(const Image& image);
            void SetNegativeX(const Image& image);
            void SetPositiveY(const Image& image);
            void SetNegativeY(const Image& image);
            void SetPositiveZ(const Image& image);
            void SetNegativeZ(const Image& image);
            static Ref<CubeMap> Create(const Image& px,const Image& nx,const Image& py,const Image& ny,const Image& pz,const Image& nz)
            {
                return CreateRef<CubeMap>(px,nx,py,ny,pz,nz);
            }
            static Ref<CubeMap> LoadFromDir(const std::string& dir);
            
            RendererId GetRendererID() const { return m_RendererID; }
            static Ref<CubeMap> Create()
            {
                return CreateRef<CubeMap>();
            }
            void Bind(int slot);
            void Bind();
            void Unbind();
        private:
            
            RendererId m_RendererID;
    };
}