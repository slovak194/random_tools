
add_executable(test_config_asio_zmq test_config_asio_zmq.cpp)
target_include_directories(test_config_asio_zmq PUBLIC ../include)
target_link_libraries(test_config_asio_zmq PUBLIC remote_config)

add_executable(test_json_eigen_map test_json_eigen_map.cpp)
target_include_directories(test_json_eigen_map PUBLIC ../include#[[ ${EIGEN3_INCLUDE_DIRS}]])
target_link_libraries(test_json_eigen_map PUBLIC remote_config)

add_executable(test_types test_types.cpp)
target_include_directories(test_types PUBLIC
        ../include
        /home/slovak/json/include
        )
target_link_libraries(test_types PUBLIC remote_config)

add_executable(test_handmade test_handmade.cpp)

