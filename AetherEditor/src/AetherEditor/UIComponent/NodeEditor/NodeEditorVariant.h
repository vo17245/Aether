#pragma once
#include <Variant>
#include <string>
#include "NodeEditorObject.h"
#include <span>
#include <Core/Core.h>
namespace AetherEditor::ImGuiComponent
{
template <typename T>
struct TypedFloatSpan
{
    T value;
};
using ScalarfSpan = TypedFloatSpan<std::span<float, 1>>;
using Vec2fSpan = TypedFloatSpan<std::span<float, 2>>;
using Vec3fSpan = TypedFloatSpan<std::span<float, 3>>;
using Vec4fSpan = TypedFloatSpan<std::span<float, 4>>;
using Mat2fSpan = TypedFloatSpan<std::span<float, 4>>;
using Mat3fSpan = TypedFloatSpan<std::span<float, 9>>;
using Mat4fSpan = TypedFloatSpan<std::span<float, 16>>;
using QuatfSpan = TypedFloatSpan<std::span<float, 4>>;
using NodeEditorVariantTypeArray =
    Aether::TypeArray<std::monostate,  std::string*,  NodeEditorObject*, ScalarfSpan, Vec2fSpan, Vec3fSpan,
              Vec4fSpan, Mat2fSpan, Mat3fSpan, Mat4fSpan, QuatfSpan>;
using NodeEditorVariant = typename Aether::TypeArrayToVariant<NodeEditorVariantTypeArray>::Type;
enum class NodeEditorVariantType
{
    Null,
    String,
    Object,
    Scalarf,
    Vec2f,
    Vec3f,
    Vec4f,
    Mat2f,
    Mat3f,
    Mat4f,
    Quatf,
};
template<typename T>
inline constexpr NodeEditorVariantType GetNodeEditorVariantTypeEnum()
{
    if constexpr (std::is_same_v<T, std::monostate>)
        return NodeEditorVariantType::Null;
    else if constexpr (std::is_same_v<T, std::string*>)
        return NodeEditorVariantType::String;
    else if constexpr (std::is_same_v<T, NodeEditorObject*>)
        return NodeEditorVariantType::Object;
    else if constexpr (std::is_same_v<T, ScalarfSpan>)
        return NodeEditorVariantType::Scalarf;
    else if constexpr (std::is_same_v<T, Vec2fSpan>)
        return NodeEditorVariantType::Vec2f;
    else if constexpr (std::is_same_v<T, Vec3fSpan>)
        return NodeEditorVariantType::Vec3f;
    else if constexpr (std::is_same_v<T, Vec4fSpan>)
        return NodeEditorVariantType::Vec4f;
    else if constexpr (std::is_same_v<T, Mat2fSpan>)
        return NodeEditorVariantType::Mat2f;
    else if constexpr (std::is_same_v<T, Mat3fSpan>)
        return NodeEditorVariantType::Mat3f;
    else if constexpr (std::is_same_v<T, Mat4fSpan>)
        return NodeEditorVariantType::Mat4f;
    else if constexpr (std::is_same_v<T, QuatfSpan>)
        return NodeEditorVariantType::Quatf;
    else
        static_assert(Aether::always_false_v<T>, "Unsupported type");
    return NodeEditorVariantType::Null;
}
} // namespace Aether::ImGuiComponent