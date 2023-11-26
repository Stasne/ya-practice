cmake_minimum_required(VERSION 3.11)

project(bookypedia CXX)
set(CMAKE_CXX_STANDARD 20)

include(${CMAKE_BINARY_DIR}/conanbuildinfo_multi.cmake)
conan_basic_setup(TARGETS)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

add_subdirectory(src/lib)

add_executable(bookypedia
	src/bookypedia.cpp
	src/bookypedia.h
	src/main.cpp
)
target_link_libraries(bookypedia PRIVATE CONAN_PKG::boost libbookypedia)

option(BUILD_TESTS "Build project tests" OFF)
if (BUILD_TESTS)
	file (GLOB_RECURSE TESTS_SRC tests/*.cpp)
	add_executable(tests ${TESTS_SRC})
	target_link_libraries(tests PRIVATE CONAN_PKG::catch2 libbookypedia)
endif(BUILD_TESTS)
