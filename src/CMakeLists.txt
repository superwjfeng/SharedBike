project(shared_bike)
add_subdirectory(common)

include_directories(./common ../third/include)

link_directories(../lib)
link_directories(../third/lib/iniparser)
link_directories(../third/lib/log4cpp)

aux_source_directory(${PROJECT_SOURCE_DIR} SOURCE_FILES)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -rdynamic -Wall -m64 -pipe -std=c++0x -lrt -Wno-reorder -Wdeprecated-declarations")

add_executable(shared_bike ${SOURCE_FILES})

target_link_libraries(shared_bike iniparser common log4cpp pthread)

