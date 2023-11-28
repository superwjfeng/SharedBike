# 搜集所有在指令路径下的原文件的文件名，将输出结果列表存储在指定的变量中
aux_source_directory(. SOURCE_COMMON_FILES)

# 生成库
add_library(common ${SOURCE_COMMON_FILES})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -rdynamic -Wall -m64 -pipe -std=c++0x -lrt -Wno-reorder -Wdeprecated-declarations")

# 指定头文件和库文件的搜索目录
include_directories(../../third/include)
link_directories(../../third/lib/iniparser)
link_directories(../../third/lib/log4cpp)

# 链接库
target_link_libraries(common iniparser)
target_link_libraries(common log4cpp) # TODO: 这里链接的到底是动态库还是静态库？
target_link_libraries(common pthread)
target_link_libraries(common dl)