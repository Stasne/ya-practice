cmake_minimum_required(VERSION 3.11)

project(game_server CXX)
set(CMAKE_CXX_STANDARD 20)

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup(TARGETS)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

file (GLOB_RECURSE SOURCES 	"src/*.cpp" "src/*.h")

add_executable(${PROJECT_NAME} ${SOURCES})
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_LIST_DIR}/src/utils 
                                                   ${CMAKE_CURRENT_LIST_DIR}/src/net 
                                                   ${CMAKE_CURRENT_LIST_DIR}/src/game)
                                                   message(STATUS "dsadsad ${CMAKE_CURRENT_LIST_DIR}/net ")

target_include_directories(${PROJECT_NAME} PRIVATE CONAN_PKG::boost)
target_link_libraries(${PROJECT_NAME} PRIVATE Threads::Threads
                                              CONAN_PKG::boost)
