set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(server_protobuf.exe server.cc bike.pb.cc)
target_link_libraries(server_protobuf.exe event protobuf)
add_executable(client_protobuf.exe client.cc bike.pb.cc)
target_link_libraries(client_protobuf.exe event protobuf)