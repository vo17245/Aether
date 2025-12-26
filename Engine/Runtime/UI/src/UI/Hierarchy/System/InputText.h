#pragma once
#include "System.h"
#include "../Component/Base.h"
#include "../Component/InputText.h"
#include "../Component/Text.h"
#include <type_traits>
#include "Debug/Log.h"
namespace Aether::UI
{
class InputTextSystem : public SystemI
{
public:
    using View = decltype(std::declval<World>().Select<BaseComponent, TextComponent, InputTextComponent>());
    struct Dispatcher
    {
        InputTextSystem& system;
        World& scene;
        View& view;
        template <typename T>
        void operator()(T& event)
        {
        }
        template <>
        void operator()(MousePositionEvent& event)
        {
            system.m_MousePosition = event.GetPosition();
        }
        template <>
        void operator()(MouseButtonPressedEvent& event)
        {
            for (const auto& [entity, base, text, inputText] : view.each())
            {
                auto& pos = base.position;
                if (pos.x() >= base.position.x() && pos.x() <= base.position.x() + base.size.x() && pos.y() >= base.position.y() && pos.y() <= base.position.y() + base.size.y())
                {
                    inputText.focus = true;
                }
                else
                {
                    inputText.focus = false;
                }
            }
        }
        template <>
        void operator()(CharacterInputEvent& event)
        {
            uint32_t u8;
            uint32_t size;
            for (const auto& [entity, base, text, inputText] : view.each())
            {
                if (inputText.focus)
                {
                    size = Unicode2Utf8(event.GetCode(), u8);
                    text.content.insert(text.content.end(), (char*)&u8, (char*)&u8 + size);
                }
            }
        }
        template <>
        void operator()(KeyboardPressEvent& event)
        {
            if (event.GetCode() == KeyboardCode::KEY_BACKSPACE)
            {
                uint32_t u8;
                uint32_t size;
                for (const auto& [entity, base, text, inputText] : view.each())
                {
                    if (inputText.focus && !text.content.empty())
                    {
                        U32String u32(text.content);
                        if (!u32.GetData().empty())
                        {
                            u32.GetData().pop_back();
                            text.content.clear();
                            for (auto u : u32.GetData())
                            {
                                size = Unicode2Utf8(u, u8);
                                text.content.insert(text.content.end(), (char*)&u8, (char*)&u8 + size);
                            }
                        }
                    }
                }
            }
        }
        template <>
        void operator()(KeyboardRepeatEvent& event)
        {
            if (event.GetCode() == KeyboardCode::KEY_BACKSPACE)
            {
                uint32_t u8;
                uint32_t size;
                for (const auto& [entity, base, text, inputText] : view.each())
                {
                    if (inputText.focus && !text.content.empty())
                    {
                        U32String u32(text.content);
                        if (!u32.GetData().empty())
                        {
                            u32.GetData().pop_back();
                            text.content.clear();
                            for (auto u : u32.GetData())
                            {
                                size = Unicode2Utf8(u, u8);
                                text.content.insert(text.content.end(), (char*)&u8, (char*)&u8 + size);
                            }
                        }
                    }
                }
            }
        }
    };

public:
    virtual void OnEvent(Event& event, World& scene)
    {
        View view = scene.Select<BaseComponent, TextComponent, InputTextComponent>();
        Dispatcher dispatcher{*this, scene, view};
        std::visit(dispatcher, event);
    }

private:
    Vec2f m_MousePosition;
};
} // namespace Aether::UI