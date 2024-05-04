#pragma once
#include "Aether/Core/Assert.h"
#include "Aether/Core/Log.h"
#include "Aether/Render/IndexBuffer.h"
#include "Aether/Render/OpenGLApi.h"
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
        hasBind = true;
        //bind index/vertex buffer(submit data to gpu)
        for(auto& bufferView:bufferViews)
        {
            bufferView.Bind();
        }
        //vertex array per primitive
        for(auto& primitive:primitives)
        {

            primitive.Bind();//create vao
            primitive.GetVertexArray().Bind();//vao bind
            /*
            * vertex location order
            * POSITION
            * NORMAL
            * TEXCOORD_N
            */
            size_t location=0;
            //set attribute
            if (primitive.HasPosition())
            {
                
                Accessor& accessor = accessors[primitive.GetAttributes()["POSITION"]];
                bufferViews[accessor.GetBufferView()].Bind();
                //POSITION accessor element type must be vec3
                if (accessor.GetElementType() == ElementType::VEC3)
                {
                    glEnableVertexAttribArray(static_cast<GLuint>(location));
                    glVertexAttribPointer(static_cast<GLuint>(location),
                        static_cast<GLint>(3),
                        static_cast<GLenum>(GL_FLOAT),
                        GL_FALSE,
                        static_cast<GLsizei>(accessor.GetStride()),
                        (const void*)accessor.GetOffset());

                }
                else if (accessor.GetElementType() == ElementType::VEC2)
                {
                    glEnableVertexAttribArray(static_cast<GLuint>(location));
                    glVertexAttribPointer(static_cast<GLuint>(location),
                        static_cast<GLint>(2),
                        static_cast<GLenum>(GL_FLOAT),
                        GL_FALSE,
                        static_cast<GLsizei>(accessor.GetStride()),
                        (const void*)accessor.GetOffset());
                }
                else
                {
                    AETHER_ASSERT(false && "invalid POSITION element type");
                    AETHER_DEBUG_LOG("invalid POSITION element type");
                    return;
                }
                location++;
            }
            if (primitive.HasNormal())
            {
                Accessor& accessor = accessors[primitive.GetAttributes()["NORMAL"]];
                bufferViews[accessor.GetBufferView()].Bind();
                //NORMAL accessor element type must be vec3
                if (accessor.GetElementType() != ElementType::VEC3)
                {
                    AETHER_ASSERT(false && "NORMAL accessor element type must be vec3");
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
            for(const auto& [key,value]:primitive.GetAttributes())
            {
                if (key.compare("NORMAL") == 0 || key.compare("POSITION") == 0)
                {
                    continue;
                }
                Accessor& accessor=accessors[value];
                bufferViews[accessor.GetBufferView()].Bind();
                //bind buffer
                auto& curBufferView=bufferViews[accessor.GetBufferView()];
                curBufferView.Bind();
                
                if(key.compare("TEXCOORD_0")==0)
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
        hasBind = false;
        for(auto& primitive:primitives)
        {
            primitive.Unbind();
        }
        for(auto& bufferView:bufferViews)
        {
            bufferView.Unbind();
        }
    }
    void Model::Render()
    {
        for(auto& p:primitives)
        {
            //get indices cnt
            auto opt_accessorIndex=p.GetIndices();
            AETHER_ASSERT(opt_accessorIndex&&"no indices draw not implment");
            auto& indicesAccessor=accessors[opt_accessorIndex.value()];
            auto& indicesBufferView=bufferViews[indicesAccessor.GetBufferView()];
            size_t indicesCnt=(indicesBufferView.GetEnd()-indicesBufferView.GetBegin())/
            indicesAccessor.GetStride();
            //draw
            OpenGLApi::DrawElements(p.GetVertexArray(), *indicesBufferView.GetIndexBuffer(), indicesCnt);
            
        }
    }
}