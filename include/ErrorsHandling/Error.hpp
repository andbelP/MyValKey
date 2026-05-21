#pragma once

#include <string>

namespace keyval::error {

enum class ErrorCode {
    kInvalidCommand,
    kKeyNotFound,
    kWrongType,
    kUndefinedError,
    kMaxMemoryExceeded,
    kKeyAlreadyExists
};

struct Error {
    ErrorCode code;
    std::string description;
};

}  // namespace keyval::error
