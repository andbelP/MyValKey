#pragma once

#include <string>

namespace keyval::error{

    enum class ErrorCode {
        kInvalidCommand,
        kKeyNotFound,
        kWrongType,
        kEngineError
    };

    struct Error {
        ErrorCode code;
        std::string description; 
    };
    
}