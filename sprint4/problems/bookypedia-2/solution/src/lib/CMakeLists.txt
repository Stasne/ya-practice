
project(bookypedia_lib CXX)
set(CMAKE_CXX_STANDARD 20)

# include(${CMAKE_BINARY_DIR}/conanbuildinfo_multi.cmake)
# conan_basic_setup(TARGETS)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

file (GLOB_RECURSE LIB_SRC src/*.cpp src/*.h)
add_library(libbookypedia STATIC ${LIB_SRC})
target_include_directories(libbookypedia PUBLIC src)
target_link_libraries(libbookypedia PUBLIC CONAN_PKG::boost Threads::Threads CONAN_PKG::fmt CONAN_PKG::libpq CONAN_PKG::libpqxx)