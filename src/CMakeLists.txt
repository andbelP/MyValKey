add_library(keyval 
    Engine/DBEngine.cpp
    Glob/GlobMatcher.cpp
    ValKeyInterpreter/ValKeyInterpreter.cpp
)
target_include_directories(keyval PUBLIC 
    ${PROJECT_SOURCE_DIR}/include
)
target_include_directories(keyval PRIVATE 
    ${PROJECT_SOURCE_DIR}/src
)