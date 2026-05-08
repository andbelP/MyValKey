#pragma once

#include <string>

namespace keyval::error{

    enum class ErrorCode{
        kInvalidCommand
    };

    struct Error {
        ErrorCode code;
        std::string description; 
    };
    
}