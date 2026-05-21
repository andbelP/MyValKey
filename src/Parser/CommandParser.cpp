#include "CommandParser.hpp"

#include <sstream>
#include <string>

#include "CommandResult/CommandResult.hpp"

namespace keyval {

ValKeyCommand CommandParser::Parse(const std::string& line) {
    ValKeyCommand command;
    std::stringstream stream(line);
    stream >> command.type;
    std::string arg;
    while (stream >> arg) {
        command.args.push_back(arg);
    }
    return command;
}

}  // namespace keyval
