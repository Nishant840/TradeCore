cmake_minimum_required(VERSION 3.15)
project(TradeCoreTests)

set(CMAKE_CXX_STANDARD 17)

find_package(Threads REQUIRED)

add_executable(tests
    catch_amalgamated.cpp
    test_matching.cpp
    ../src/OrderBook.cpp
    ../src/MatchingEngine.cpp
    ../src/Metrics.cpp
    ../src/EngineRunner.cpp
)

target_include_directories(tests PRIVATE ../)
target_link_libraries(tests PRIVATE Threads::Threads)