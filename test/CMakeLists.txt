
add_executable(test_config_asio_zmq test_config_asio_zmq.cpp)
target_include_directories(test_config_asio_zmq PUBLIC ../include)
target_link_libraries(test_config_asio_zmq PUBLIC remote_config)

add_executable(test_json_eigen_map test_json_eigen_map.cpp)
target_include_directories(test_json_eigen_map PUBLIC ../include#[[ ${EIGEN3_INCLUDE_DIRS}]])
target_link_libraries(test_json_eigen_map PUBLIC remote_config)

