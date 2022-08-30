link_libraries(
        ${Boost_LIBRARIES}
        ${YAML_CPP_LIBRARIES}
        pthread
        nlohmann_json::nlohmann_json
        zmq
)

include_directories(
        ../include
        ${EIGEN3_INCLUDE_DIRS}
        ${YAML_CPP_INCLUDE_DIR}
)

add_executable(test_config_server test_config_server.cpp)
target_compile_definitions(test_config_server PUBLIC PROJECT_SOURCE_DIR="${PROJECT_SOURCE_DIR}")

add_executable(test_rpc_server test_rpc_server.cpp)
