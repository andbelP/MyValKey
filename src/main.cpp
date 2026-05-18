#include <iostream>
#include "ValKeyInterpreter/ValKeyInterpreter.hpp"
#include "Engine/DBEngine.hpp"

int main() {

    keyval::DBEngine engine;
    keyval::ValKeyInterpreter interpreter(engine);
    std::string line;
    while (std::getline(std::cin, line)) {
        if(line.empty()){
            continue;
        }
        if(line=="EXIT"){
            break;
        }
        //TODO
    }
}