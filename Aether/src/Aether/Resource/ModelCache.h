#pragma once
#include <filesystem>
#include <string>
#include <unordered_map>
#include "Model/Model.h"
#include "Model/ModelLoader.h"
namespace Aether
{
    class ModelCache
    {
    public:
        ~ModelCache()=default;
        static ModelCache& GetInstance()
        {
            static ModelCache instance;
            return instance;
        }
        static Ref<Model> LoadFromFile(const std::string& path)
        {
            return GetInstance().LoadFromFileImpl(path);
        }

    private:
        Ref<Model> LoadFromFileImpl(const std::string& path)
        {
            ClearCacheIfNecessary();
            auto iter=m_Cache.find(path);
            if(iter==m_Cache.end())
            {
                auto model=ModelLoader::LoadFromFile(path);
                if(model)
                {
                    m_Cache[path]=model;
                    return model;
                }
                else 
                {
                    return nullptr;
                }
            }
            else 
            {
                return iter->second;
            }
        }
        void ClearCacheIfNecessary()
        {
            if(m_Cache.size()>m_MaxCacheSize)
            {
                m_Cache.clear();
            }
        }
        ModelCache()=default;
        std::unordered_map<std::string,Ref<Model>> m_Cache;
        size_t m_MaxCacheSize=16;


    };
}