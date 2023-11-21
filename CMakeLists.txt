cmake_minimum_required(VERSION 3.0)

aux_source_directory(. SOURCE_COMMON_FILES)

add_library(common ${SOURCE_COMMON_FILES})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -rdynamic -Wall -m64 -pipe -std=c++0x -lrt -Wno-reorder -Wdeprecated-declaration")

include_directories(../../third/include)

link_directories(../../third/lib/iniparser)

target_link_libraries(common iniparser)

target_link_libraries(common dl)