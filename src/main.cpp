#include <iostream>

#include "Engine/DBEngine.hpp"
#include "Parser/CommandParser.hpp"
#include "ValKeyInterpreter/ValKeyInterpreter.hpp"

int main() {
    keyval::DBEngine engine;
    keyval::ValKeyInterpreter interpreter(engine);
    std::string line;
    while (std::getline(std::cin, line)) {
        if (line.empty()) {
            continue;
        }
        if (line == "EXIT") {
            break;
        }
        auto command = keyval::CommandParser::Parse(line);
        auto result = interpreter.Interpret(command);
        
        if (std::holds_alternative<keyval::ValKeyError>(result)) {

            std::cerr << "Error: "
                      << std::get<keyval::ValKeyError>(result).description
                      << std::endl;

        } 
        else if (std::holds_alternative<keyval::Ok>(result)) {

            std::cout << "OK" << std::endl;

        } 
        else if (std::holds_alternative<keyval::Null>(result)) {

            std::cout << "(null)" << std::endl;

        } 
        else if (std::holds_alternative<std::string>(result)) {

            std::cout << std::get<std::string>(result) << std::endl;

        } 
        else if (std::holds_alternative<std::vector<std::string>>(result)) {

            auto& values = std::get<std::vector<std::string>>(result);
            for (std::size_t i = 0; i < values.size(); ++i) {
                std::cout << i + 1 << ") " << values[i] << "\n";
            }

        }
    }
}