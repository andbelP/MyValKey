#include "ValKeyInterpreter/ValKeyInterpreter.hpp"




namespace keyval {

ValKeyResult ValKeyInterpreter::Set(std::span<const std::string> args) {
    if (args.size() != 2) {
        return ValKeyError{"invalid args cnt"};
    }

    auto result = engine_.Set(args[0], args[1]);
    if (!result.has_value()) {
        if (result.error().code == error::ErrorCode::kMaxMemoryExceeded) {
            return ValKeyError{"OOM command not allowed when used memory > 'maxmemory'"};
        }
        return ValKeyError{result.error().description};
    }

    return Ok{};
}

ValKeyResult ValKeyInterpreter::Get(std::span<const std::string> args) {
    if (args.size() != 1) {
        return ValKeyError{"invalid args cnt"};
    }

    auto result = engine_.Get(args[0]);
    if (!result.has_value()) {
        if (result.error().code == error::ErrorCode::kKeyNotFound) {
            return Null{};
        }
        return ValKeyError{result.error().description};
    }

    return result.value();
}

ValKeyResult ValKeyInterpreter::StrLen(std::span<const std::string> args) {
    if (args.size() != 1) {
        return ValKeyError{"invalid args cnt"};
    }

    auto result = engine_.StrLen(args[0]);
    if (!result.has_value()) {
        if (result.error().code == error::ErrorCode::kKeyNotFound) {
            return std::string("0");
        }
        return ValKeyError{result.error().description};
    }

    return std::to_string(result.value());
}

ValKeyResult ValKeyInterpreter::Append(std::span<const std::string> args) {
    if (args.size() != 2) {
        return ValKeyError{"invalid args cnt"};
    }

    auto result = engine_.Append(args[0], args[1]);
    if (!result.has_value()) {
        if (result.error().code == error::ErrorCode::kMaxMemoryExceeded) {
            return ValKeyError{"OOM command not allowed when used memory > 'maxmemory'"}; // TODO поменять названия ошибок в DBENGINE чтобы не делать тут костыли.
        }
        return ValKeyError{result.error().description};
    }

    return std::to_string(engine_.StrLen(args[0]).value());

}

}