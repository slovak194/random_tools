cmake_minimum_required(VERSION 3.13)
project(remote-tools)

option(BUILD_TESTS "add test subdir" 1)

set(CMAKE_CXX_STANDARD 20)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-psabi")

find_package(Eigen3 QUIET NO_MODULE)
find_package(Boost QUIET COMPONENTS system chrono regex)
find_package(nlohmann_json REQUIRED)
find_package(yaml-cpp QUIET)

if(${yaml-cpp_FOUND})
    add_definitions(-DYAML_SUPPORT)
endif(${yaml-cpp_FOUND})

include_directories(include/remote_tools)

if (${BUILD_TESTS})
    add_subdirectory(test)
endif (${BUILD_TESTS})
