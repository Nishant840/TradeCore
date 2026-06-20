cmake_minimum_required(VERSION 3.15)
project(TradeCoreEngine)

set(CMAKE_CXX_STANDARD 17)

find_package(Threads REQUIRED)

add_executable(tradecore_engine
    src/main.cpp
    src/OrderBook.cpp
    src/MatchingEngine.cpp
    src/Metrics.cpp
    src/EngineRunner.cpp
    src/NetworkServer.cpp
)

target_include_directories(tradecore_engine PRIVATE include third_party)
target_link_libraries(tradecore_engine PRIVATE Threads::Threads)