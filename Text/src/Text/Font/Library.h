#pragma once
#include "Stroke.h"
namespace Aether::Text
{
class Library
{
public:
    FT_Library handle = nullptr;
    FT_Stroker stroker = nullptr;
    Library(const Library&) = delete;
    Library& operator=(const Library&) = delete;

    Library(Library&& other)
    {
        handle = other.handle;
        other.handle = nullptr;
        stroker = other.stroker;
        other.stroker = nullptr;
    }
    Library& operator=(Library&& other)
    {
        if (handle)
        {
            FT_Done_FreeType(handle);
        }
        handle = other.handle;
        other.handle = nullptr;
        if (stroker)
        {
            FT_Stroker_Done(stroker);
        }
        stroker = other.stroker;
        other.stroker = nullptr;
        return *this;
    }
    ~Library()
    {
        if (handle)
        {
            FT_Done_FreeType(handle);
        }
    }
    static std::optional<Library> Create()
    {
        FT_Library library;
        FT_Error error = FT_Init_FreeType(&library);
        if (error)
        {
            return std::nullopt;
        }
        Library context;
        context.handle = library;
        auto res = FT_Stroker_New(library, &context.stroker);
        if (res)
        {
            FT_Done_FreeType(library);
            return std::nullopt;
        }
        return context;
    }
    void SetStrokerParam(const StrokeParam& param)
    {
        FT_Stroker_Set(stroker, param.radius,
                       (FT_Stroker_LineCap_)param.lineCap, (FT_Stroker_LineJoin)param.lineJoin,
                       param.miterLimit);
    }

private:
    Library() = default;
};
} // namespace Aether::Text