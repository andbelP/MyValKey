#pragma once

#include <string>
#include <optional>
#include <vector>

namespace keyval {

struct ValKeyCommand{
    std::string type;
    std::vector<std::string> args;
};

struct ValKeyResult{
    std::optional<std::string> to_cout;
    std::optional<std::string> to_cerr;
};

} // namespace keyval