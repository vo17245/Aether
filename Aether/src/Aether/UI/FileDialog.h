#pragma once
#include "../ImGui/ImGui"
#include <filesystem>
#include <optional>
#include <regex>
#include <vector>
namespace Aether
{
    class FileDialog
    {
    public:
        enum class FileType
        {
            Regular,
            Directory,
        };
        struct Filter
        {
            std::optional<FileType> type;
            std::optional<std::regex> name;
        };
    public:
        template<typename RootDir,typename ShowFilter,typename SelectFilter>
        FileDialog(RootDir&& rootDir,
        ShowFilter&& showFilter,
        SelectFilter&& selectFilter)
            :m_ShowFilter(std::forward<ShowFilter>(showFilter)),
            m_SelectFilter(std::forward<SelectFilter>(selectFilter)),
            m_RootDir(std::forward<RootDir>(rootDir))
        {
            static_assert(std::is_same_v<RootDir, std::filesystem::path>);
            static_assert(std::is_same_v<ShowFilter, std::vector<Filter>> );
            static_assert(std::is_same_v<SelectFilter, std::vector<Filter>> );
        }
    private:
        std::vector<Filter> m_ShowFilter;
        std::vector<Filter> m_SelectFilter;
        std::filesystem::path m_RootDir;
    
    };
}