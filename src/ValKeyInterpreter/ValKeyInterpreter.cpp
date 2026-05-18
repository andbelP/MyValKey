#include "ValKeyInterpreter.hpp"

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <exception>
#include <expected>
#include <string>
#include <unordered_set>
#include <vector>

namespace keyval {

namespace {

std::string ToUpper(std::string value) {
    for (auto& ch : value) {
        ch = std::toupper(ch);
    }
    return value;
}

}  // namespace

ValKeyResult ValKeyInterpreter::Interpret(const ValKeyCommand& command) {
    auto command_type = ToUpper(command.type);
    auto it = methods_.find(command_type);

    if (it == methods_.end()) {
        return ValKeyError{"unknown command " + command.type};
    }

    return (this->*(it->second))(command.args);
}

ValKeyResult ValKeyInterpreter::Del(std::span<const std::string> args) {
    if (args.empty()) {
        return ValKeyError("invalid args cnt");
    }

    std::size_t deleted = 0;

    for (const auto& key : args) {
        auto result = engine_.Del(key);

        if (result.has_value()) {
            ++deleted;
            continue;
        }

        if (result.error().code != error::ErrorCode::kKeyNotFound) {
            return ValKeyError{result.error().description};
        }
    }

    return std::to_string(deleted);
}

ValKeyResult ValKeyInterpreter::Exists(std::span<const std::string> args) {
    if (args.empty()) {
        return ValKeyError("invalid args cnt");
    }

    std::size_t count = 0;

    for (const auto& key : args) {
        auto result = engine_.Exists(key);
        if (!result.has_value()) {
            return ValKeyError{result.error().description};
        }

        if (result.value()) {
            ++count;
        }
    }

    return std::to_string(count);
}

ValKeyResult ValKeyInterpreter::Type(std::span<const std::string> args) {
    if (args.size() != 1) {
        return ValKeyError("invalid args cnts");
    }

    auto result = engine_.Type(args[0]);
    if (!result.has_value()) {
        if (result.error().code == error::ErrorCode::kKeyNotFound) {
            return std::string("none");
        }
        return ValKeyError{result.error().description};
    }

    switch (result.value()) {
        case StorageType::kString:
            return "string";
        case StorageType::kList:
            return "list";
        case StorageType::kSet:
            return "set";
    }

    return "undefined type";
}

}  // namespace keyval