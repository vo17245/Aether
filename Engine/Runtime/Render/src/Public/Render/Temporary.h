#pragma once
#include <tuple>
#include "Core/TypeArray.h"
#include "Vulkan/RenderContext.h"
#include <vector>
namespace Aether
{
template <typename... Ts>
class Temporary
{
public:
    using Types = TypeArray<Ts...>;
    template <typename T>
        requires IsArrayContainType<T, Types>::value
    void Push(T* t)
    {
        std::get<TypeIndexInArray<T, Types>::value>(m_Data).push_back(t);
    }
    template <typename T>
        requires IsArrayContainType<T, Types>::value
    void Clear()
    {
        auto& arr = std::get<TypeIndexInArray<T, Types>::value>(m_Data);
        for (auto& p : arr)
        {
            delete p;
        }
        arr.clear();
    }
    void ClearAll()
    {
        (Clear<Ts>(), ...);
    }
    template<typename T>
        requires IsArrayContainType<T, Types>::value
    std::vector<T*>& Get()
    {
        return std::get<TypeIndexInArray<T, Types>::value>(m_Data);
    }
    ~Temporary()
    {
        ClearAll();
    }

private:
    std::tuple<std::vector<Ts*>...> m_Data;
};
} // namespace Aether