add_library(keyval 
    Glob/GlobMatcher.cpp
    ValKeyInterpreter/ValKeyInterpreter.cpp
    Engine/DBEngineGeneral.cpp
    Engine/DBEngineString.cpp
    Engine/DBEngineList.cpp
    Engine/DBEngineSet.cpp
)
target_include_directories(keyval PUBLIC 
    ${PROJECT_SOURCE_DIR}/include
)
target_include_directories(keyval PRIVATE 
    ${PROJECT_SOURCE_DIR}/src
)