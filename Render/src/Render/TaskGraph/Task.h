#pragma once
#include "TaskBase.h"
#include <variant>
#include "RenderTask.h"
#include "TaskArray.h"
namespace Aether::TaskGraph 
{
    using Task = std::variant<std::monostate, Borrow<RenderTaskBase>,Borrow<RenderTaskArray>>;
}