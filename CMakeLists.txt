cmake_minimum_required(VERSION 3.13)
project(remote-config)

set(CMAKE_CXX_STANDARD 20)

find_package(Eigen3 REQUIRED NO_MODULE)
find_package(Boost REQUIRED COMPONENTS system chrono regex)
find_package(nlohmann_json REQUIRED)
#find_package(Flatbuffers REQUIRED)

add_library(remote_config src/Server.cpp)
target_include_directories(remote_config PUBLIC
        ${EIGEN3_INCLUDE_DIRS}
        include/remote_config)

target_link_libraries(remote_config PUBLIC
        ${Boost_LIBRARIES}
        pthread
        nlohmann_json::nlohmann_json
        zmq
        )


add_subdirectory(test)
