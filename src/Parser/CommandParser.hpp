#pragma once

#include "CommandResult/CommandResult.hpp"

namespace keyval {

class CommandParser {
   public:
    static ValKeyCommand Parse(const std::string& line);
};

}  // namespace keyval
