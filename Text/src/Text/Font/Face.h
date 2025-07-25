#pragma once
#include "Library.h"
namespace Aether::Text
{
    class Face
{
public:
    Face(const Face&) = delete;
    Face(Face&& other)
    {
        handle = other.handle;
        other.handle = nullptr;
        m_FontFileData = std::move(other.m_FontFileData);
    }
    Face& operator=(const Face&) = delete;
    Face& operator=(Face&& other)
    {
        if (handle)
        {
            FT_Done_Face(handle);
        }
        handle = other.handle;
        other.handle = nullptr;
        m_FontFileData = std::move(other.m_FontFileData);
        return *this;
    }
    FT_Face handle = nullptr;
    ~Face()
    {
        if (handle)
        {
            FT_Done_Face(handle);
        }
    }
    static std::optional<Face> Create(Library& context, const char* path)
    {
        Face face;
        auto fontfileOpt = Filesystem::ReadFile(path);
        if (!fontfileOpt)
        {
            return std::nullopt;
        }
        face.m_FontFileData = std::move(fontfileOpt.value());
        auto& fontFile = face.m_FontFileData;
        // FT_Error error = FT_New_Face(context.handle, path, 0, &face.handle);
        FT_Error error = FT_New_Memory_Face(context.handle,
                                            fontFile.data(), fontFile.size(), 0, &face.handle);
        if (error)
        {
            return std::nullopt;
        }
        return face;
    }

private:
    Face() = default;
    std::vector<uint8_t> m_FontFileData;
};
}