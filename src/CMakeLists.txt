project(shared_bike)
add_subdirectory(util)

# target_include_directories(shared_bike_server.exe ./util ../third/include)
# target_include_directories(shared_bike_server.exe threadpool protocol)

# target_link_directories(shared_bike_server.exe ../lib)
# target_link_directories(shared_bike_server.exe ../third/lib/iniparser)
# target_link_directories(shared_bike_server.exe ../third/lib/log4cpp)
aux_source_directory(${PROJECT_SOURCE_DIR} SOURCE_FILES)

add_executable(shared_bike_server.exe ${SOURCE_FILES}
  threadpool/thread_pool.cc threadpool/thread_cond.cc threadpool/thread_mutex.cc
  util/iniconfig.cc util/logger.cc
  protocol/bike.pb.cc
  database/sqlconnection.cc database/QueryProcessor.cc database/user_service.cc
)

# TODO: find_package(MySQL REQUIRED)

# shared_bike_server.exe的包含目录
target_include_directories(shared_bike_server.exe
    PRIVATE ./util
    PRIVATE ../third/include
    PRIVATE threadpool
    PRIVATE protocol
    # ${MySQL_INCLUDE_DIRS}
    PRIVATE database
)

# shared_bike_server.exe的链接目录
target_link_directories(shared_bike_server.exe
    PRIVATE ../lib
    PRIVATE ../third/lib/iniparser
    PRIVATE ../third/lib/log4cpp
)


set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -rdynamic -Wall -m64 -pipe -std=c++0x -lrt -Wno-reorder -Wdeprecated-declarations")

# target_link_libraries(shared_bike iniparser common log4cpp pthread)
target_link_libraries(shared_bike_server.exe protobuf event pthread iniparser log4cpp mysqlclient)

add_subdirectory(client)
add_subdirectory(database)


