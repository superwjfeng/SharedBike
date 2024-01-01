project(shared_bike)
add_subdirectory(util)

include_directories(./util ../third/include)
include_directories(threadpool protocol)

link_directories(../lib)
link_directories(../third/lib/iniparser)
link_directories(../third/lib/log4cpp)

aux_source_directory(${PROJECT_SOURCE_DIR} SOURCE_FILES)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -rdynamic -Wall -m64 -pipe -std=c++0x -lrt -Wno-reorder -Wdeprecated-declarations")

add_executable(shared_bike_server.exe ${SOURCE_FILES}
  threadpool/thread_pool.cc threadpool/thread_cond.cc threadpool/thread_mutex.cc
  util/iniconfig.cc util/logger.cc
  protocol/bike.pb.cc
)
# target_link_libraries(shared_bike iniparser common log4cpp pthread)
target_link_libraries(shared_bike_server.exe protobuf event pthread iniparser log4cpp)

add_subdirectory(client)


