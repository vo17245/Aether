#pragma once
#include <variant>

namespace Aether {
namespace Reflection {
template <typename T> struct Meta {};
template <typename... Args> struct ComponentTypeGroup {};
template <typename T> struct ComponentType {};
template <typename Handler> void ForEachComponentType(Handler &handler) {}
template <typename Handler, typename T, typename... Args>
void ForEachComponentType(Handler &handler) {
  handler(ComponentType<T>{});
  ForEachComponentType<Handler, Args...>(handler);
}
template <typename... Args, typename Handler>
void ForEachComponentType(ComponentTypeGroup<Args...>, Handler &handler) {
  ForEachComponentType<Handler, Args...>(handler);
}

template<typename ComponentT,typename Handler>
void ForEachField(ComponentT& obj, Handler handler)
{
	for (size_t i = 0;i < Meta<ComponentT>::field_count;i++)
	{
		handler(Meta<ComponentT>::field_names[i], //name const char*
			Meta<ComponentT>::field_types[i],//type const char*
			Meta<ComponentT>::field_comments[i],//comment const char*
			Meta<ComponentT>::Get(obj, Meta<ComponentT>::field_names[i])//field std::variant
		);
	}
}
template<typename ComponentT>
const char* GetComponentTypeString()
{
	return Meta<ComponentT>::name;
}

} // namespace Reflection
} // namespace Aether