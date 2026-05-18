#pragma once

#include <string>
#include <optional>
#include <vector>
#include <variant>

namespace keyval {

struct ValKeyCommand{
    std::string type;
    std::vector<std::string> args;
};


struct Ok{};
struct Null{};
struct ValKeyError{
    std::string description;
};


using ValKeyResult = std::variant<Ok, Null, std::string, std::vector<std::string>, ValKeyError>;

} // namespace keyval