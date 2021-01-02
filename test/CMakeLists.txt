
include_directories(
        ${PROJECT_SOURCE_DIR}/third_party/moteus
        ${PROJECT_SOURCE_DIR}/third_party/BNO055_driver-master
)

link_libraries(
        ${Boost_LIBRARIES}
        pthread
        nlohmann_json::nlohmann_json
        flatbuffers
        zmq
)

add_executable(test_config_asio_zmq test_config_asio_zmq.cpp)
