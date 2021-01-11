
add_executable(test_server test_server.cpp)
target_include_directories(test_server PUBLIC
        ../include
        ${EIGEN3_INCLUDE_DIRS}
        )
target_link_libraries(test_server PUBLIC
        ${Boost_LIBRARIES}
        pthread
        nlohmann_json::nlohmann_json
        zmq
        )
target_compile_definitions(test_server PUBLIC USE_ZMQ)
target_compile_definitions(test_server PUBLIC PROJECT_SOURCE_DIR="${PROJECT_SOURCE_DIR}")
target_link_libraries(test_server PUBLIC zmq)
