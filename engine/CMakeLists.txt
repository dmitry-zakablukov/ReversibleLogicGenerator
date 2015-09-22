cmake_minimum_required(VERSION 2.8)
project(engine)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
set(SOURCE_LIB
    BooleanEdgeSearcher.cpp 
    CompositeGenerator.cpp
    Cycle.cpp
    Element.cpp
    Exceptions.cpp
    GtGenerator.cpp
    GtGeneratorWithMemory.cpp
    IniParser.cpp
    PartialGtGenerator.cpp
    PartialResultParams.cpp
    Permutation.cpp
    PermutationUtils.cpp
    PostProcessor.cpp
    ProgramOptions.cpp
    Range.cpp
    RmGenerator.cpp
    RmSpectraUtils.cpp
    SchemeUtils.cpp
    std.cpp
    TfcFormatter.cpp
    Timer.cpp
    Transposition.cpp
    TruthTableParser.cpp
    TruthTableUtils.cpp
    utils.cpp
    Values.cpp
)
add_library(engine STATIC ${SOURCE_LIB})

