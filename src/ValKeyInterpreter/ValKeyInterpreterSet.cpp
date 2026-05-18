#include "ValKeyInterpreter.hpp"

namespace keyval {

ValKeyResult ValKeyInterpreter::SAdd(std::span<const std::string> args) {
    if (args.size() < 2) {
        return ValKeyError("invalid args cnt");
    }

    std::size_t added = 0;

    for (std::size_t i = 1; i < args.size(); ++i) {
        auto existed = engine_.SIsMember(args[0], args[i]);

        if (existed.has_value() && existed.value()) {
            continue;
        }

        if (!existed.has_value() &&
            existed.error().code != error::ErrorCode::kKeyNotFound) {
            return ValKeyError{existed.error().description};
        }

        auto result = engine_.SAdd(args[0], args[i]);
        if (!result.has_value()) {
            if (result.error().code == error::ErrorCode::kMaxMemoryExceeded) {
                return ValKeyError{"OOM command not allowed when used memory > 'maxmemory'"};
            }
            return ValKeyError{result.error().description};
        }

        ++added;
    }

    return std::to_string(added);
}

ValKeyResult ValKeyInterpreter::SRem(std::span<const std::string> args) {
    if (args.size() < 2) {
        return ValKeyError("invalid args cnt");
    }

    std::size_t removed = 0;

    for (std::size_t i = 1; i < args.size(); ++i) {
        auto existed = engine_.SIsMember(args[0], args[i]);

        if (!existed.has_value()) {
            if (existed.error().code == error::ErrorCode::kKeyNotFound) {
                continue;
            }
            return ValKeyError{existed.error().description};
        }

        if (!existed.value()) {
            continue;
        }

        auto result = engine_.SRem(args[0], args[i]);
        if (!result.has_value()) {
            return ValKeyError{result.error().description};
        }

        ++removed;
    }

    return std::to_string(removed);
}

ValKeyResult ValKeyInterpreter::SIsMember(std::span<const std::string> args) {
    if (args.size() != 2) {
        return ValKeyError("invalid args cnt");
    }

    auto result = engine_.SIsMember(args[0], args[1]);
    if (!result.has_value()) {
        if (result.error().code == error::ErrorCode::kKeyNotFound) {
            return std::string("0");
        }
        return ValKeyError{result.error().description};
    }

    return result.value() ? std::string("1") : std::string("0");
}

ValKeyResult ValKeyInterpreter::SMembers(std::span<const std::string> args) {
    if (args.size() != 1) {
        return ValKeyError("invalid args cnt");
    }

    auto result = engine_.SMembers(args[0]);
    if (!result.has_value()) {
        if (result.error().code == error::ErrorCode::kKeyNotFound) {
            return std::vector<std::string>{};
        }
        return ValKeyError{result.error().description};
    }

    return std::vector<std::string>(result.value().begin(), result.value().end());
}

ValKeyResult ValKeyInterpreter::SCard(std::span<const std::string> args) {
    if (args.size() != 1) {
        return ValKeyError("invalid args cnt");
    }

    auto result = engine_.SCard(args[0]);
    if (!result.has_value()) {
        if (result.error().code == error::ErrorCode::kKeyNotFound) {
            return std::string("0");
        }
        return ValKeyError{result.error().description};
    }

    return std::to_string(result.value());
}

ValKeyResult ValKeyInterpreter::SUnion(std::span<const std::string> args) {
    if (args.empty()) {
        return ValKeyError("invalid args cnt");
    }

    auto result = engine_.SUnion(args);
    if (!result.has_value()) {
        if (result.error().code == error::ErrorCode::kKeyNotFound) {
            return std::vector<std::string>{};
        }
        return ValKeyError{result.error().description};
    }

    return std::vector<std::string>(result.value().begin(), result.value().end());
}

ValKeyResult ValKeyInterpreter::SInter(std::span<const std::string> args) {
    if (args.empty()) {
        return ValKeyError("invalid args cnt");
    }

    auto result = engine_.SInter(args);
    if (!result.has_value()) {
        if (result.error().code == error::ErrorCode::kKeyNotFound) {
            return std::vector<std::string>{};
        }
        return ValKeyError{result.error().description};
    }

    return std::vector<std::string>(result.value().begin(), result.value().end());
}

ValKeyResult ValKeyInterpreter::SDiff(std::span<const std::string> args) {
    if (args.empty()) {
        return ValKeyError("invalid args cnt");
    }

    auto result = engine_.SDiff(args);
    if (!result.has_value()) {
        return ValKeyError{result.error().description};
    }

    return std::vector<std::string>(result.value().begin(), result.value().end());
}

ValKeyResult ValKeyInterpreter::SMove(std::span<const std::string> args) {
    if (args.size() != 3) {
        return ValKeyError("invalid args cnt");
    }

    auto result = engine_.SMove(args[0], args[1], args[2]);
    if (!result.has_value()) {
        if (result.error().code == error::ErrorCode::kKeyNotFound ||
            result.error().code == error::ErrorCode::kInvalidCommand) {
            return ValKeyError("0");
        }

        if (result.error().code == error::ErrorCode::kMaxMemoryExceeded) {
            return ValKeyError{"OOM command not allowed when used memory > 'maxmemory'"};
        }

        return ValKeyError{result.error().description};
    }

    return std::string("1");
}

}