#pragma once
#include <variant>

namespace Aether
{
    namespace Reflection
    {
        template<typename T>
        struct Meta{};
        template<typename... Args>
        struct ComponentTypeSet{};
        template<typename T>
        struct ComponentType{};
        template<typename Handler>
        void ForEachComponentType(Handler& handler) {}
        template<typename Handler, typename T, typename... Args>
        void ForEachComponentType(Handler& handler) {
            handler(ComponentType<T>{});
            ForEachComponentType<Handler, Args...>(handler);
        }
        template<typename... Args,typename Handler >
        void ForEachComponentType(ComponentTypeSet<Args...>,Handler& handler) {
            ForEachComponentType<Handler, Args...>(handler);
        }
    }
}