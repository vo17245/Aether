#pragma once
#include <concepts>
#include <tuple>
#include "ImageView.h"
#include <array>
#include <utility>
#include <vector>
#include "Semaphore.h"
namespace Aether {
namespace vk {

template <typename T>
concept IsImageView = std::is_same_v<T, ImageView>;

template <typename... Ts>
concept AreAllImageView = (IsImageView<Ts> && ...);
template <typename T>
concept HasGetHandle = requires(T a) {
    {
        a.GetHandle()
    };
};
template <typename... Ts>
concept AreSame = (std::is_same_v<Ts, Ts> && ...);

template <typename T>
concept IsStdVector = std::is_same_v<T, std::vector<typename T::value_type, typename T::allocator_type>>;

template <typename... Ts>
constexpr auto make_array(Ts... args)
{
    return std::array{args...};
}
template <typename... Ts>
constexpr auto make_handle_array(const Ts&... ts)
{
    return make_array(ts.GetHandle()...);
}

template <AreAllImageView... ImageViews>
constexpr auto make_image_view_handle_array(const ImageViews&... imageViews)
{
    return make_handle_array<ImageView>(imageViews...);
}



template <typename T, typename... Ts>
auto merge_vector(const T& first, const Ts&... ts)
{
    std::vector<typename T::value_type> result;
    result.insert(result.end(), first.begin(), first.end());
    (result.insert(result.end(), ts.begin(), ts.end()), ...);
    return result;
}

template <typename... Ts>
auto merge_vertex_attribute_description(const Ts&... ts)
{
    return merge_vector(ts.CreateVulkanAttributeDescriptions()...);
}
template <typename T>
concept IsSemaphore = std::is_same_v<T, Semaphore>;

template <typename... Ts>
concept AreAllSemaphore = (IsSemaphore<Ts> && ...);
}
} // namespace Aether::vk