cmake_minimum_required(VERSION 3.11)

project(book_manager CXX)
set(CMAKE_CXX_STANDARD 20)

include(${CMAKE_BINARY_DIR}/conanbuildinfo_multi.cmake)
conan_basic_setup(TARGETS)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

file (GLOB_RECURSE SOURCES
        "src/*.cpp" "src/*.h")

add_executable(${PROJECT_NAME} main.cpp ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE src)
target_include_directories(${PROJECT_NAME} PRIVATE CONAN_PKG::boost)
target_link_libraries(${PROJECT_NAME} CONAN_PKG::libpqxx CONAN_PKG::boost)


# TESTS
option (BUILD_TESTS "build tests" OFF)
file (GLOB_RECURSE TESTS_SRC "tests/*.cpp")
add_executable(tests ${TESTS_SRC} ${SOURCES} )

target_include_directories(tests PRIVATE src)

target_link_libraries(tests PRIVATE CONAN_PKG::gtest
                                    CONAN_PKG::libpqxx
                                    CONAN_PKG::boost)
