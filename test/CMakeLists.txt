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
add_executable(test_rpc_client test_rpc_client.cpp)
add_executable(test_rpc_async_server_and_client test_rpc_async_server_and_client.cpp)

add_executable(test_pub test_pub.cpp)
add_executable(test_sub test_sub.cpp)

add_executable(doctest_json doctest_json.cpp)
add_executable(doctest_json_yaml doctest_json_yaml.cpp)
