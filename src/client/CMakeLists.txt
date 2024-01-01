add_executable(client.exe client.cc ../protocol/bike.pb.cc)
target_link_libraries(client.exe event protobuf log4cpp)