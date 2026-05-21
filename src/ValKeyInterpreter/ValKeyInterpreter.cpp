#include "ValKeyInterpreter.hpp"

#include <chrono>
#include <cstddef>
#include <span>
#include <string>
#include <unordered_map>
#include <vector>

#include "CommandResult/CommandResult.hpp"
#include "Engine/StorageTypes.hpp"
#include "ErrorsHandling/Error.hpp"
#include "Utils.hpp"

namespace keyval {

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

ValKeyResult ValKeyInterpreter::Expire(std::span<const std::string> args) {
    if (args.size() != 2) {
        return ValKeyError("invalid args cnt");
    }

    auto ttl_result = ParseInt(std::string(args[1]));
    if (!ttl_result.has_value() || ttl_result.value() < 0) {
        return ValKeyError{ttl_result.error()};
    }
    auto result =
        engine_.Expire(args[0], std::chrono::seconds(ttl_result.value()));
    if (!result.has_value()) {
        return ValKeyError{result.error().description};
    }
    return Ok{};
}

ValKeyResult ValKeyInterpreter::TTL(std::span<const std::string> args) {
    if (args.size() != 1) {
        return ValKeyError("invalid args cnt");
    }

    auto result = engine_.GetTTL(args[0]);
    if (!result.has_value()) {
        if (result.error().code == error::ErrorCode::kKeyNotFound) {
            return std::to_string(-2);
        }
        return ValKeyError{result.error().description};
    }

    if (!result.value().has_value()) {
        return std::to_string(-1);
    }

    return std::to_string(result.value().value());
}

ValKeyResult ValKeyInterpreter::Keys(std::span<const std::string> args) {
    if (args.size() != 1) {
        return ValKeyError("invalid args cnt");
    }

    auto result = engine_.Keys(args[0]);
    if (!result.has_value()) {
        return ValKeyError{result.error().description};
    }

    std::vector<std::string> keys(result.value().begin(), result.value().end());
    return keys;
}

ValKeyResult ValKeyInterpreter::FlushDB(std::span<const std::string> args) {
    if (!args.empty()) {
        return ValKeyError("invalid args cnt");
    }

    engine_.FlushDb();
    return Ok{};
}

ValKeyResult ValKeyInterpreter::DbSize(std::span<const std::string> args) {
    if (!args.empty()) {
        return ValKeyError("invalid args cnt");
    }

    auto result = engine_.EntryCount();

    return std::to_string(result);
}

ValKeyResult ValKeyInterpreter::MemoryUsage(std::span<const std::string> args) {
    if (args.size() != 2 || ToUpper(args[0]) != "USAGE") {
        return ValKeyError("invalid command. try MEMORY USAGE <key>");
    }

    auto result = engine_.MemoryUsageOfKey(args[1]);
    if (!result.has_value()) {
        return Null{};
    }

    return std::to_string(result.value());
}

ValKeyResult ValKeyInterpreter::Config(std::span<const std::string> args) {
    static const std::unordered_map<std::string,
                                    ValKeyResult (ValKeyInterpreter::*)(
                                        std::span<const std::string> args)>
        kMethods{{"MAXMEMORY", &ValKeyInterpreter::ConfigMaxMemory}};

    if (args.size() < 2) {
        return ValKeyError{"undefined command"};
    }

    auto it = kMethods.find(ToUpper(args[1]));
    if (it == kMethods.end()) {
        return ValKeyError{"unknown config parameter: " + args[1]};
    }

    return (this->*(it->second))(args);
}

ValKeyResult ValKeyInterpreter::ConfigMaxMemory(
    std::span<const std::string> args) {
    if (args.size() == 2 && ToUpper(args[0]) == "GET" &&
        ToUpper(args[1]) == "MAXMEMORY") {
        if (engine_.GetMaxMemoryUsage().has_value()) {
            return std::to_string(engine_.GetMaxMemoryUsage().value());
        } else {
            return ValKeyError{"maxmemory is not set"};
        }
    } else if (args.size() == 3 && ToUpper(args[0]) == "SET" &&
               ToUpper(args[1]) == "MAXMEMORY") {
        auto memory_result = ParseMemorySize(args[2]);
        if (!memory_result.has_value()) {
            return ValKeyError{memory_result.error()};
        }

        auto result = engine_.SetMaxMemoryUsage(memory_result.value());
        if (!result.has_value()) {
            return ValKeyError{result.error().description};
        }
        return Ok{};
    } else {
        return ValKeyError{
            "invalid command. try CONFIG GET maxmemory or CONFIG SET maxmemory "
            "<cnt>"};
    }
}

}  // namespace keyval
