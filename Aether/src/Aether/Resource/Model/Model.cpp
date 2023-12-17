#pragma once
#include "Aether/Core/Assert.h"
#include "Aether/Core/Log.h"
#include "Aether/Resource/Model/Accessor.h"
#include "Aether/Utils/TarjanAlgorithm.h"
#include "Model.h"
namespace Aether
{
    bool Model::HasCycle()
    {
        auto graph=TarjanAlgorithm(nodes.size());
        for(size_t i=0;i<nodes.size();i++)
        {
            auto& node=nodes[i];
            for(const auto& child:node.children)
            {
                graph.AddEdge(i, child);
            }
        }
        auto sccs=graph.Tarjan();
        return sccs.size()!=0;
    }
    void Model::Bind()
    {
        //bind vertex buffer
        for(auto& bufferView:bufferViews)
        {
            bufferView.Bind();
        }
        //vertex array per primitive
        for(auto& primitive:primitives)
        {
            primitive.Bind();
            primitive.GetVertexArray().Bind();
            /*
            * vertex location order
            * POSITION
            * NORMAL
            * TEXCOORD_N
            */
            size_t location=0;
            for(const auto& [key,value]:primitive.GetAttributes())
            {
                Accessor& accessor=accessors[value];
                if(key.compare("POSITION")==0)
                {
                    //POSITION accessor element type must be vec3
                    if(accessor.GetElementType()!=ElementType::VEC3)
                    {
                        AETHER_ASSERT(false&&"POSITION accessor element type must be vec3");
                        AETHER_DEBUG_LOG("POSITION accessor element type not vec3");
                        return;
                    }
                    glEnableVertexAttribArray(static_cast<GLuint>(location));
                    glVertexAttribPointer(static_cast<GLuint>(location), 
				        static_cast<GLint>(3),
				        static_cast<GLenum>(GL_FLOAT),
				        GL_FALSE, 
				        static_cast<GLsizei>(accessor.GetStride()), 
				        (const void*)accessor.GetOffset());
                    location++;
                }
                else if(key.compare("NORMAL")==0)
                {
                    //NORMAL accessor element type must be vec3
                    if(accessor.GetElementType()!=ElementType::VEC3)
                    {
                        AETHER_ASSERT(false&&"NORMAL accessor element type must be vec3");
                        AETHER_DEBUG_LOG("NORMAL accessor element type not vec3");
                        return;
                    }
                    glEnableVertexAttribArray(static_cast<GLuint>(location));
                    glVertexAttribPointer(static_cast<GLuint>(location), 
				        static_cast<GLint>(3),
				        static_cast<GLenum>(GL_FLOAT),
				        GL_FALSE, 
				        static_cast<GLsizei>(accessor.GetStride()), 
				        (const void*)accessor.GetOffset());
                    location++;
                }
                else if(key.compare("TEXCOORD_0"))
                {
                    //TEXCOORD_0 accessor element type must be vec2
                    if(accessor.GetElementType()!=ElementType::VEC2)
                    {
                        AETHER_ASSERT(false&&"TEXCOORD_0 accessor element type must be vec2");
                        AETHER_DEBUG_LOG_ERROR("TEXCOORD_0 accessor element type not vec2");
                        return;
                    }
                    glEnableVertexAttribArray(static_cast<GLuint>(location));
                    glVertexAttribPointer(static_cast<GLuint>(location), 
				        static_cast<GLint>(2),
				        static_cast<GLenum>(GL_FLOAT),
				        GL_FALSE, 
				        static_cast<GLsizei>(accessor.GetStride()), 
				        (const void*)accessor.GetOffset());
                    location++;
                }
                else 
                {
                    AETHER_ASSERT(false&&"unknown attribute");
                    AETHER_DEBUG_LOG_ERROR("unknown attribute {}");
                    return;
                }
                
            }
        }

    }
    void Model::Unbind()
    {
        for(auto& primitive:primitives)
        {
            primitive.Unbind();
        }
        for(auto& bufferView:bufferViews)
        {
            bufferView.Unbind();
        }
    }
}