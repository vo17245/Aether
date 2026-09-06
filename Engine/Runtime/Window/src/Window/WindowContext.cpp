#include "WindowContext.h"

#include "KeyboardEvent.h"
#include "MouseEvent.h"
#include "Window/Event.h"
#include "Window/Window.h"
#include <ImGui/Backend/imgui_impl_sdl3.h>
#include <ImGui/Core/imgui.h>
#include <Render/Threads/SubmitThread.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>
#include <cassert>
#include <stdexcept>

namespace Aether
{
namespace
{
KeyboardCode SdlKeyToKeyboardCode(SDL_Keycode key, SDL_Scancode scancode)
{
    switch (scancode)
    {
    case SDL_SCANCODE_KP_0: return KeyboardCode::KEY_KP_0;
    case SDL_SCANCODE_KP_1: return KeyboardCode::KEY_KP_1;
    case SDL_SCANCODE_KP_2: return KeyboardCode::KEY_KP_2;
    case SDL_SCANCODE_KP_3: return KeyboardCode::KEY_KP_3;
    case SDL_SCANCODE_KP_4: return KeyboardCode::KEY_KP_4;
    case SDL_SCANCODE_KP_5: return KeyboardCode::KEY_KP_5;
    case SDL_SCANCODE_KP_6: return KeyboardCode::KEY_KP_6;
    case SDL_SCANCODE_KP_7: return KeyboardCode::KEY_KP_7;
    case SDL_SCANCODE_KP_8: return KeyboardCode::KEY_KP_8;
    case SDL_SCANCODE_KP_9: return KeyboardCode::KEY_KP_9;
    case SDL_SCANCODE_KP_PERIOD: return KeyboardCode::KEY_KP_DECIMAL;
    case SDL_SCANCODE_KP_DIVIDE: return KeyboardCode::KEY_KP_DIVIDE;
    case SDL_SCANCODE_KP_MULTIPLY: return KeyboardCode::KEY_KP_MULTIPLY;
    case SDL_SCANCODE_KP_MINUS: return KeyboardCode::KEY_KP_SUBTRACT;
    case SDL_SCANCODE_KP_PLUS: return KeyboardCode::KEY_KP_ADD;
    case SDL_SCANCODE_KP_ENTER: return KeyboardCode::KEY_KP_ENTER;
    case SDL_SCANCODE_KP_EQUALS: return KeyboardCode::KEY_KP_EQUAL;
    default: break;
    }
    if (key >= SDLK_A && key <= SDLK_Z)
        return static_cast<KeyboardCode>('A' + (key - SDLK_A));
    if ((key >= SDLK_0 && key <= SDLK_9) || (key >= SDLK_SPACE && key <= 0x60))
        return static_cast<KeyboardCode>(key);

    switch (key)
    {
    case SDLK_ESCAPE: return KeyboardCode::KEY_ESCAPE;
    case SDLK_RETURN: return KeyboardCode::KEY_ENTER;
    case SDLK_TAB: return KeyboardCode::KEY_TAB;
    case SDLK_BACKSPACE: return KeyboardCode::KEY_BACKSPACE;
    case SDLK_INSERT: return KeyboardCode::KEY_INSERT;
    case SDLK_DELETE: return KeyboardCode::KEY_DELETE;
    case SDLK_RIGHT: return KeyboardCode::KEY_RIGHT;
    case SDLK_LEFT: return KeyboardCode::KEY_LEFT;
    case SDLK_DOWN: return KeyboardCode::KEY_DOWN;
    case SDLK_UP: return KeyboardCode::KEY_UP;
    case SDLK_PAGEUP: return KeyboardCode::KEY_PAGE_UP;
    case SDLK_PAGEDOWN: return KeyboardCode::KEY_PAGE_DOWN;
    case SDLK_HOME: return KeyboardCode::KEY_HOME;
    case SDLK_END: return KeyboardCode::KEY_END;
    case SDLK_CAPSLOCK: return KeyboardCode::KEY_CAPS_LOCK;
    case SDLK_SCROLLLOCK: return KeyboardCode::KEY_SCROLL_LOCK;
    case SDLK_NUMLOCKCLEAR: return KeyboardCode::KEY_NUM_LOCK;
    case SDLK_PRINTSCREEN: return KeyboardCode::KEY_PRINT_SCREEN;
    case SDLK_PAUSE: return KeyboardCode::KEY_PAUSE;
    case SDLK_F1: return KeyboardCode::KEY_F1;
    case SDLK_F2: return KeyboardCode::KEY_F2;
    case SDLK_F3: return KeyboardCode::KEY_F3;
    case SDLK_F4: return KeyboardCode::KEY_F4;
    case SDLK_F5: return KeyboardCode::KEY_F5;
    case SDLK_F6: return KeyboardCode::KEY_F6;
    case SDLK_F7: return KeyboardCode::KEY_F7;
    case SDLK_F8: return KeyboardCode::KEY_F8;
    case SDLK_F9: return KeyboardCode::KEY_F9;
    case SDLK_F10: return KeyboardCode::KEY_F10;
    case SDLK_F11: return KeyboardCode::KEY_F11;
    case SDLK_F12: return KeyboardCode::KEY_F12;
    case SDLK_F13: return KeyboardCode::KEY_F13;
    case SDLK_F14: return KeyboardCode::KEY_F14;
    case SDLK_F15: return KeyboardCode::KEY_F15;
    case SDLK_F16: return KeyboardCode::KEY_F16;
    case SDLK_F17: return KeyboardCode::KEY_F17;
    case SDLK_F18: return KeyboardCode::KEY_F18;
    case SDLK_F19: return KeyboardCode::KEY_F19;
    case SDLK_F20: return KeyboardCode::KEY_F20;
    case SDLK_F21: return KeyboardCode::KEY_F21;
    case SDLK_F22: return KeyboardCode::KEY_F22;
    case SDLK_F23: return KeyboardCode::KEY_F23;
    case SDLK_F24: return KeyboardCode::KEY_F24;
    case SDLK_LSHIFT: return KeyboardCode::KEY_LEFT_SHIFT;
    case SDLK_LCTRL: return KeyboardCode::KEY_LEFT_CONTROL;
    case SDLK_LALT: return KeyboardCode::KEY_LEFT_ALT;
    case SDLK_LGUI: return KeyboardCode::KEY_LEFT_SUPER;
    case SDLK_RSHIFT: return KeyboardCode::KEY_RIGHT_SHIFT;
    case SDLK_RCTRL: return KeyboardCode::KEY_RIGHT_CONTROL;
    case SDLK_RALT: return KeyboardCode::KEY_RIGHT_ALT;
    case SDLK_RGUI: return KeyboardCode::KEY_RIGHT_SUPER;
    case SDLK_APPLICATION: return KeyboardCode::KEY_MENU;
    default: return KeyboardCode::KEY_UNKNOWN;
    }
}

bool SdlButtonToMouseButtonCode(Uint8 button, MouseButtonCode& code)
{
    switch (button)
    {
    case SDL_BUTTON_LEFT: code = MouseButtonCode::Left; return true;
    case SDL_BUTTON_RIGHT: code = MouseButtonCode::Right; return true;
    case SDL_BUTTON_MIDDLE: code = MouseButtonCode::Middle; return true;
    default: return false;
    }
}

void PushUtf8Characters(Window& window, const char* text)
{
    const auto* cursor = reinterpret_cast<const unsigned char*>(text);
    while (*cursor != 0)
    {
        uint32_t codepoint = 0;
        size_t length = 0;
        if ((*cursor & 0x80u) == 0)
        {
            codepoint = *cursor;
            length = 1;
        }
        else if ((*cursor & 0xe0u) == 0xc0u)
        {
            codepoint = *cursor & 0x1fu;
            length = 2;
        }
        else if ((*cursor & 0xf0u) == 0xe0u)
        {
            codepoint = *cursor & 0x0fu;
            length = 3;
        }
        else if ((*cursor & 0xf8u) == 0xf0u)
        {
            codepoint = *cursor & 0x07u;
            length = 4;
        }
        else
        {
            ++cursor;
            continue;
        }

        bool valid = true;
        for (size_t i = 1; i < length; ++i)
        {
            if ((cursor[i] & 0xc0u) != 0x80u)
            {
                valid = false;
                break;
            }
            codepoint = (codepoint << 6u) | (cursor[i] & 0x3fu);
        }
        if (!valid)
        {
            ++cursor;
            continue;
        }
        window.PushEvent(CharacterInputEvent(codepoint));
        cursor += length;
    }
}
} // namespace

bool WindowContext::Init()
{
    return SDL_Init(SDL_INIT_VIDEO);
}

void WindowContext::Shutdown()
{
    Get().m_Windows.clear();
    SDL_Quit();
}

void WindowContext::Register(SDL_Window* handle, Window* instance)
{
    assert(handle != nullptr && instance != nullptr);
    Get().m_Windows[SDL_GetWindowID(handle)] = instance;
    SDL_StartTextInput(handle);
}

void WindowContext::Remove(SDL_Window* handle)
{
    if (handle != nullptr)
        Get().m_Windows.erase(SDL_GetWindowID(handle));
}

Window* WindowContext::FindWindow(SDL_WindowID id)
{
    const auto iterator = Get().m_Windows.find(id);
    return iterator == Get().m_Windows.end() ? nullptr : iterator->second;
}

void WindowContext::PollEvents()
{
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        if (ImGui::GetCurrentContext() != nullptr)
            ImGui_ImplSDL3_ProcessEvent(&event);
        HandleEvent(event);
    }
}

void WindowContext::HandleEvent(const SDL_Event& event)
{
    if (event.type == SDL_EVENT_QUIT)
    {
        for (auto& [id, window] : Get().m_Windows)
            window->m_ShouldClose = true;
        return;
    }

    SDL_WindowID windowId = 0;
    switch (event.type)
    {
    case SDL_EVENT_WINDOW_RESIZED:
    case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED:
    case SDL_EVENT_WINDOW_MINIMIZED:
    case SDL_EVENT_WINDOW_RESTORED:
    case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
        windowId = event.window.windowID;
        break;
    case SDL_EVENT_KEY_DOWN:
    case SDL_EVENT_KEY_UP:
        windowId = event.key.windowID;
        break;
    case SDL_EVENT_TEXT_INPUT:
        windowId = event.text.windowID;
        break;
    case SDL_EVENT_MOUSE_MOTION:
        windowId = event.motion.windowID;
        break;
    case SDL_EVENT_MOUSE_BUTTON_DOWN:
    case SDL_EVENT_MOUSE_BUTTON_UP:
        windowId = event.button.windowID;
        break;
    default:
        return;
    }

    Window* window = FindWindow(windowId);
    if (window == nullptr)
        return;

    switch (event.type)
    {
    case SDL_EVENT_WINDOW_RESIZED:
        window->PushEvent(WindowResizeEvent(event.window.data1, event.window.data2));
        break;
    case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED:
        window->PushEvent(FrameBufferResizeEvent(event.window.data1, event.window.data2));
        HandleFramebufferResize(*window, event.window.data1, event.window.data2);
        break;
    case SDL_EVENT_WINDOW_MINIMIZED:
        window->m_Minilized = true;
        break;
    case SDL_EVENT_WINDOW_RESTORED:
        window->m_Minilized = false;
        break;
    case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
        window->m_ShouldClose = true;
        break;
    case SDL_EVENT_KEY_DOWN:
    {
        const auto code = SdlKeyToKeyboardCode(event.key.key, event.key.scancode);
        if (event.key.repeat)
            window->PushEvent(KeyboardRepeatEvent(code));
        else
            window->PushEvent(KeyboardPressEvent(code));
        break;
    }
    case SDL_EVENT_KEY_UP:
        window->PushEvent(KeyboardReleaseEvent(SdlKeyToKeyboardCode(event.key.key, event.key.scancode)));
        break;
    case SDL_EVENT_TEXT_INPUT:
        PushUtf8Characters(*window, event.text.text);
        break;
    case SDL_EVENT_MOUSE_MOTION:
        window->PushEvent(MousePositionEvent(Vec2f(event.motion.x, event.motion.y)));
        break;
    case SDL_EVENT_MOUSE_BUTTON_DOWN:
    {
        MouseButtonCode code;
        if (!SdlButtonToMouseButtonCode(event.button.button, code))
            break;
        auto input = MouseButtonPressedEvent(code);
        input.SetPosition(Vec2f(event.button.x, event.button.y));
        window->PushEvent(input);
        break;
    }
    case SDL_EVENT_MOUSE_BUTTON_UP:
    {
        MouseButtonCode code;
        if (!SdlButtonToMouseButtonCode(event.button.button, code))
            break;
        auto input = MouseButtonReleasedEvent(code);
        input.SetPosition(Vec2f(event.button.x, event.button.y));
        window->PushEvent(input);
        break;
    }
    default:
        break;
    }
}

void WindowContext::HandleFramebufferResize(Window& window, int width, int height)
{
    if (width == 0 || height == 0)
    {
        window.m_Minilized = true;
        return;
    }

    window.m_Minilized = false;
    Render::SubmitThread::WaitIdle();
    window.ImGuiWindowContextDestroy();
    window.ReleaseRenderObject();
    if (!window.CreateRenderObject())
    {
        assert(false && "failed to recreate window Vulkan resources");
        return;
    }
    window.CreateRenderGraph();
    window.ImGuiWindowContextInit();
}

Vec2i WindowContext::MainMonitorSize()
{
    SDL_Rect bounds{};
    const SDL_DisplayID display = SDL_GetPrimaryDisplay();
    if (display == 0 || !SDL_GetDisplayBounds(display, &bounds))
        return Vec2i(0, 0);
    return Vec2i(bounds.w, bounds.h);
}

std::vector<const char*> WindowContext::RequiredVulkanInstanceExtensions()
{
    Uint32 count = 0;
    const char* const* names = SDL_Vulkan_GetInstanceExtensions(&count);
    if (names == nullptr)
        throw std::runtime_error(SDL_GetError());
    return std::vector<const char*>(names, names + count);
}
} // namespace Aether
