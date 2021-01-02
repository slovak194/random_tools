cmake_minimum_required(VERSION 3.13)
project(awesomeconfig)

set(CMAKE_CXX_STANDARD 20)

find_package(Eigen3 REQUIRED NO_MODULE)
find_package(Boost REQUIRED COMPONENTS system chrono regex)
find_package(nlohmann_json REQUIRED)
find_package(Flatbuffers REQUIRED)

include_directories(
        include
        ${EIGEN3_INCLUDE_DIRS}
)

add_subdirectory(test)
