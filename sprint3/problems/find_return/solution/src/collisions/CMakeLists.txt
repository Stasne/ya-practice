cmake_minimum_required(VERSION 3.11)

set(CMAKE_CXX_STANDARD 20)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

add_library(collision_detection_lib STATIC
	"include/collisions/collision_detector.h"
	"src/collision_detector.cpp"
)
target_include_directories(collision_detection_lib PUBLIC include)
target_link_libraries(collision_detection_lib PUBLIC CONAN_PKG::boost Threads::Threads)

