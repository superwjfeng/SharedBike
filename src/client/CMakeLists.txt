add_executable(client.exe client.cc ../protocol/bike.pb.cc)
target_include_directories(client.exe 
  PRIVATE ../util
  PRIVATE ../protocol
  PRIVATE ../../third/include
)
target_link_directories(client.exe PRIVATE ../../third/lib/log4cpp)
target_link_libraries(client.exe event protobuf log4cpp)