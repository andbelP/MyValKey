#include "ValKeyInterpreter.hpp"
#include "Utils.hpp"

namespace keyval {

ValKeyResult ValKeyInterpreter::LPush(std::span<const std::string> args) {
    if (args.size() < 2) {
        return ValKeyError("invalid args cnt");
    }

    for (std::size_t i = 1; i < args.size(); ++i) {
        auto result = engine_.LPush(args[0], args[i]);
        if (!result.has_value()) {
            if (result.error().code == error::ErrorCode::kMaxMemoryExceeded) {
                return ValKeyError{"OOM command not allowed when used memory > 'maxmemory'"};
            }
            return ValKeyError{result.error().description};
        }
    }

    auto len = engine_.LLen(args[0]);
    if (!len.has_value()) {
        return ValKeyError{len.error().description};
    }

    return std::to_string(len.value());
}

ValKeyResult ValKeyInterpreter::RPush(std::span<const std::string> args) {
    if (args.size() < 2) {
        return ValKeyError("invalid args cnt");
    }

    for (std::size_t i = 1; i < args.size(); ++i) {
        auto result = engine_.RPush(args[0], args[i]);
        if (!result.has_value()) {
            if (result.error().code == error::ErrorCode::kMaxMemoryExceeded) {
                return ValKeyError{"OOM command not allowed when used memory > 'maxmemory'"};
            }
            return ValKeyError{result.error().description};
        }
    }

    auto len = engine_.LLen(args[0]);
    if (!len.has_value()) {
        return ValKeyError{len.error().description};
    }

    return std::to_string(len.value());
}

ValKeyResult ValKeyInterpreter::LPop(std::span<const std::string> args) {
    if (args.size() != 1 && args.size() != 2) {
        return ValKeyError("invalid args cnt");
    }

    if (args.size() == 1) {
        auto result = engine_.LPop(args[0]);
        if (!result.has_value()) {
            if (result.error().code == error::ErrorCode::kKeyNotFound ||
                result.error().code == error::ErrorCode::kInvalidCommand) {
                return Null{};
            }
            return ValKeyError{result.error().description};
        }

        return result.value();
    }

    auto count = ParseInt(args[1]);
    if (!count.has_value()) {
        return ValKeyError{count.error()};
    }
    if(count.value()<0){
        return ValKeyError{"cnt must be positive"};
    }

    std::vector<std::string> values;
    for (std::size_t i = 0; i < count.value(); ++i) {
        auto result = engine_.LPop(args[0]);
        if (!result.has_value()) {
            if (result.error().code == error::ErrorCode::kKeyNotFound ||
                result.error().code == error::ErrorCode::kInvalidCommand) {
                break;
            }
            return ValKeyError{result.error().description};
        }

        values.push_back(result.value());
    }

    return values;
}

ValKeyResult ValKeyInterpreter::RPop(std::span<const std::string> args) {
    if (args.size() != 1 && args.size() != 2) {
        return ValKeyError("invalid args cnt");
    }

    if (args.size() == 1) {
        auto result = engine_.RPop(args[0]);
        if (!result.has_value()) {
            if (result.error().code == error::ErrorCode::kKeyNotFound ||
                result.error().code == error::ErrorCode::kInvalidCommand) {
                return Null{};
            }
            return ValKeyError{result.error().description};
        }

        return result.value();
    }

    auto count = ParseInt(args[1]);
    if (!count.has_value()) {
        return ValKeyError{count.error()};
    }
    if(count.value()<0){
        return ValKeyError{"cnt must be positive"};
    }

    std::vector<std::string> values;
    for (std::size_t i = 0; i < count.value(); ++i) {
        auto result = engine_.RPop(args[0]);
        if (!result.has_value()) {
            if (result.error().code == error::ErrorCode::kKeyNotFound ||
                result.error().code == error::ErrorCode::kInvalidCommand) {
                break;
            }
            return ValKeyError{result.error().description};
        }

        values.push_back(result.value());
    }

    return values;
}

ValKeyResult ValKeyInterpreter::LLen(std::span<const std::string> args) {
    if (args.size() != 1) {
        return ValKeyError("invalid args cnt");
    }

    auto result = engine_.LLen(args[0]);
    if (!result.has_value()) {
        if (result.error().code == error::ErrorCode::kKeyNotFound) {
            return std::string("0");
        }
        return ValKeyError{result.error().description};
    }

    return std::to_string(result.value());
}

ValKeyResult ValKeyInterpreter::LRange(std::span<const std::string> args) {
    if (args.size() != 3) {
        return ValKeyError("invalid args cnt");
    }

    auto start = ParseInt(args[1]);
    if (!start.has_value()) {
        return ValKeyError{start.error()};
    }

    auto stop = ParseInt(args[2]);
    if (!stop.has_value()) {
        return ValKeyError{stop.error()};
    }

    auto result = engine_.LRange(
        args[0],
        start.value(),
        stop.value());

    if (!result.has_value()) {
        if (result.error().code == error::ErrorCode::kKeyNotFound) {
            return std::vector<std::string>{};
        }
        return ValKeyError{result.error().description};
    }

    return result.value();
}

ValKeyResult ValKeyInterpreter::LIndex(std::span<const std::string> args) {
    if (args.size() != 2) {
        return ValKeyError("invalid args cnt");
    }

    auto index = ParseInt(args[1]);
    if (!index.has_value()) {
        return ValKeyError{index.error()};
    }

    auto result = engine_.LIndex(
        args[0],
        index.value());

    if (!result.has_value()) {
        if (result.error().code == error::ErrorCode::kKeyNotFound ||
            result.error().code == error::ErrorCode::kInvalidCommand) {
            return Null{};
        }
        return ValKeyError{result.error().description};
    }

    return result.value();
}

ValKeyResult ValKeyInterpreter::LSet(std::span<const std::string> args) {
    if (args.size() != 3) {
        return ValKeyError("invalid args cnt");
    }

    auto index = ParseInt(args[1]);
    if (!index.has_value()) {
        return ValKeyError{index.error()};
    }

    auto result = engine_.LSet(
        args[0],
        index.value(),
        args[2]);

    if (!result.has_value()) {
        if (result.error().code == error::ErrorCode::kMaxMemoryExceeded) {
            return ValKeyError{"OOM command not allowed when used memory > 'maxmemory'"};
        }
        return ValKeyError{result.error().description};
    }

    return Ok{};
}

ValKeyResult ValKeyInterpreter::LInsert(std::span<const std::string> args) {
    auto size_exp = engine_.LLen(args[0]);
    if(!size_exp){
        if(size_exp.error().code == error::ErrorCode::kKeyNotFound){
            return ValKeyError{"no such key"};
        }
        return ValKeyError{size_exp.error().description};
    }

    for(int i = 0; i < size_exp.value(); ++i){
        auto val = engine_.LIndex(args[0], i);
        if(!val){
            return ValKeyError{val.error().description};
        }
        if(val.value() == args[2]){
            if(ToUpper(args[1]) == "BEFORE"){
                auto result = engine_.LInsert(args[0], i-1, args[3]);
                if (!result.has_value()) {
                    if (result.error().code == error::ErrorCode::kMaxMemoryExceeded) {
                        return ValKeyError{"OOM command not allowed when used memory > 'maxmemory'"};
                    }
                    return ValKeyError{result.error().description};
                }
                return Ok{};
            } else if(ToUpper(args[1]) == "AFTER"){
                auto result = engine_.LInsert(args[0], i, args[3]);
                if (!result.has_value()) {
                    if (result.error().code == error::ErrorCode::kMaxMemoryExceeded) {
                        return ValKeyError{"OOM command not allowed when used memory > 'maxmemory'"};
                    }
                    return ValKeyError{result.error().description};
                }
                return Ok{};
            } else{
                return ValKeyError{"second arg must be BEFORE or AFTER"};
            }
        }
    }
    return ValKeyError{"pivot not found"};

}

}  // namespace keyval