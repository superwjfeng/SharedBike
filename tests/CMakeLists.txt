set(CMAKE_CXX_STANDARD 17)

# 获取当前源代码目录的路径
get_filename_component(PROJECT_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR} DIRECTORY)

# 设置可执行文件的输出目录为与build同级的tests目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/tests)

include_directories(../lib/util)
include_directories(../src ../src/util ../src/protocol ../third/include)

link_directories(../lib)
link_directories(../third/lib/iniparser)
link_directories(../third/lib/log4cpp)

add_executable(test_mobile_ev.exe
  test_mobile_ev.cc ../src/protocol/bike.pb.cc
  ../src/ievent.cc ../src/events_def.cc ../src/eventtype.cc
  ../src/threadpool/thread_pool.cc ../src/threadpool/thread_cond.cc ../src/threadpool/thread_mutex.cc
)

target_link_libraries(test_mobile_ev.exe gtest gtest_main pthread protobuf)