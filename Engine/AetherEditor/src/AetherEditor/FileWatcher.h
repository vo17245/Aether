#pragma once
#include <efsw/efsw.hpp>
#include "FileWatchListener.h"
#include <memory>
#include <Core/Core.h>
namespace AetherEditor
{

class FileWatcher
{
public:
    using OnChangeCallback =
        std::function<void(efsw::WatchID /*watchid*/, const std::string& /*dir*/, const std::string& /*filename*/,
                           efsw::Action /*action*/, std::string /*oldFilename*/)>;

    static FileWatcher& GetSingleton()
    {
        static FileWatcher instance;
        return instance;
    }
    static efsw::WatchID WatchDirectory(const std::string& directory, bool recursive, OnChangeCallback callback)
    {
        return GetSingleton().WatchDirectoryImpl(directory, recursive, callback);
    }
    static void RemoveWatch(efsw::WatchID watchid)
    {
        GetSingleton().RemoveWatchImpl(watchid);
    }
    static void Start()
    {
        GetSingleton().StartImpl();
    }

private:
    efsw::WatchID WatchDirectoryImpl(const std::string& directory, bool recursive, OnChangeCallback callback)
    {
        auto listener = std::make_unique<UpdateListener>(callback);
        efsw::WatchID watchid = m_Watcher->addWatch(directory, listener.get(), recursive);
        if (watchid != efsw::Errors::FileNotFound)
        {
            m_Listeners[watchid] = std::move(listener);
        }
        return watchid;
    }
    void RemoveWatchImpl(efsw::WatchID watchid)
    {
        auto it = m_Listeners.find(watchid);
        if (it != m_Listeners.end())
        {
            m_Listeners.erase(it);
        }
    }
    void StartImpl()
    {
        m_Watcher=std::make_unique<efsw::FileWatcher>();
        m_Watcher->watch();
    }

private:
    std::unordered_map<efsw::WatchID, std::unique_ptr<UpdateListener>> m_Listeners;
    std::unique_ptr<efsw::FileWatcher> m_Watcher;
};
}