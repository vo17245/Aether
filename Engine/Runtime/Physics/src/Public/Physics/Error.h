#pragma once

#include <expected>
#include <string>

namespace Aether::Physics
{

enum class ErrorCode
{
    InvalidArgument,
    InvalidConfiguration,
    InvalidHandle,
    CapacityExceeded,
    ShapeCreationFailed,
    BodyCreationFailed,
    OutOfMemory,
    BackendFailure
};

struct Error
{
    ErrorCode code = ErrorCode::BackendFailure;
    std::string message;
};

template <class T>
using Result = std::expected<T, Error>;

} // namespace Aether::Physics
