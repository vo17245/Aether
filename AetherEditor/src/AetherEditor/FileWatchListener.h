#pragma once
#include <efsw/efsw.hpp>
#include <functional>
#include <unordered_map>
class UpdateListener : public efsw::FileWatchListener
{
public:
    using OnChangeCallback =
        std::function<void(efsw::WatchID /*watchid*/, const std::string& /*dir*/, const std::string& /*filename*/,
                           efsw::Action /*action*/, std::string /*oldFilename*/)>;
    void handleFileAction(efsw::WatchID watchid, const std::string& dir, const std::string& filename,
                          efsw::Action action, std::string oldFilename) override
    {
        if (m_OnChange)
        {
            m_OnChange(watchid, dir, filename, action, oldFilename);
        }
        // switch (action)
        //{
        // case efsw::Actions::Add:
        //     std::cout << "DIR (" << dir << ") FILE (" << filename << ") has event Added" << std::endl;
        //     break;
        // case efsw::Actions::Delete:
        //     std::cout << "DIR (" << dir << ") FILE (" << filename << ") has event Delete" << std::endl;
        //     break;
        // case efsw::Actions::Modified:
        //     std::cout << "DIR (" << dir << ") FILE (" << filename << ") has event Modified" << std::endl;
        //     break;
        // case efsw::Actions::Moved:
        //     std::cout << "DIR (" << dir << ") FILE (" << filename << ") has event Moved from (" << oldFilename << ")"
        //               << std::endl;
        //     break;
        // default:
        //     std::cout << "Should never happen!" << std::endl;
        // }
    }
    UpdateListener(OnChangeCallback callback) : m_OnChange(callback)
    {
    }

private:
    OnChangeCallback m_OnChange;
};