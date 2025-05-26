#pragma once
#include <cstdint>
#include <format>
#include <variant>
#include <string>
#include "KeyboardCode.h"
#include "EventBase.h"
#include "KeyboardEvent.h"
#include "WindowEvent.h"
#include "MouseEvent.h"
namespace Aether {

using Event = std::variant<std::monostate,
                           FrameBufferResizeEvent,
                           WindowResizeEvent,
                           KeyboardReleaseEvent,
                           KeyboardPressEvent,
                           KeyboardRepeatEvent,
                           CharacterInputEvent,
                           MousePositionEvent,
                           MouseButtonPressedEvent,
                           MouseButtonReleasedEvent>;

} // namespace Aether