cmake_minimum_required(VERSION 3.11)

project(game_server CXX)
set(CMAKE_CXX_STANDARD 20)

include(${CMAKE_BINARY_DIR}/conanbuildinfo_multi.cmake)
conan_basic_setup(TARGETS)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

file (GLOB_RECURSE SOURCES 	"src/*.cpp" "src/*.h")

add_executable(${PROJECT_NAME} ${SOURCES})
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_LIST_DIR}/src/utils 
                                                   ${CMAKE_CURRENT_LIST_DIR}/src/net 
                                                   ${CMAKE_CURRENT_LIST_DIR}/src/game)


target_include_directories(${PROJECT_NAME} PRIVATE CONAN_PKG::boost)
target_link_libraries(${PROJECT_NAME} PRIVATE Threads::Threads
                                              CONAN_PKG::boost)


option(BUILD_TESTS "Build test programs" ON)

if (BUILD_TESTS)
    file (GLOB_RECURSE TEST_SOURCES
                    "tests/loot_generator_tests.cpp" 	
                    "src/game/*.cpp" "src/game/*.h"
                    "src/net/*.cpp" "src/net/*.h"
                    "src/utils/*.cpp" "src/utils/*.h")
    add_executable(game_server_tests ${TEST_SOURCES})
    target_include_directories(game_server_tests PRIVATE ${CMAKE_CURRENT_LIST_DIR}/src/utils 
                                                    ${CMAKE_CURRENT_LIST_DIR}/src/net 
                                                    ${CMAKE_CURRENT_LIST_DIR}/src/game)
    target_include_directories(game_server_tests PRIVATE  CONAN_PKG::boost)
    target_link_libraries(game_server_tests PRIVATE CONAN_PKG::catch2 CONAN_PKG::boost Threads::Threads) 
endif(BUILD_TESTS)