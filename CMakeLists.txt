cmake_minimum_required(VERSION 2.8)
project(reversible-logic-generator)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
set(SOURCE_EXE
    common.cpp
    discreteLogSynthesis.cpp
    generalSynthesis.cpp
    Gf2Field.cpp
    main.cpp
    optimizationTest.cpp
    removeNegativeLines.cpp
    std.cpp
)
include_directories(engine)
add_executable(reversible-logic-generator ${SOURCE_EXE})
add_subdirectory(engine)
target_link_libraries(reversible-logic-generator engine)

