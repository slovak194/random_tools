
add_executable(test_server test_server.cpp)
target_include_directories(test_server PUBLIC
        ../include
        ${EIGEN3_INCLUDE_DIRS}
        ${YAML_CPP_INCLUDE_DIR}
        )
target_link_libraries(test_server PUBLIC
        ${Boost_LIBRARIES}
        ${YAML_CPP_LIBRARIES}
        pthread
        nlohmann_json::nlohmann_json
        zmq
        )
#target_compile_definitions(test_server PUBLIC USE_ZMQ)
target_compile_definitions(test_server PUBLIC PROJECT_SOURCE_DIR="${PROJECT_SOURCE_DIR}")
target_link_libraries(test_server PUBLIC zmq)

add_executable(test_yaml test_yaml.cpp)
target_include_directories(test_yaml PUBLIC
        ../include
        ${EIGEN3_INCLUDE_DIRS}
        ${YAML_CPP_INCLUDE_DIR}
        )
target_link_libraries(test_yaml PUBLIC
        ${Boost_LIBRARIES}
        ${YAML_CPP_LIBRARIES}
        pthread
        nlohmann_json::nlohmann_json
        zmq
        )

add_executable(test_rpc test_rpc.cpp)

target_link_libraries(test_rpc PUBLIC
        ${Boost_LIBRARIES}
        nlohmann_json::nlohmann_json
        zmq
        pthread
        )