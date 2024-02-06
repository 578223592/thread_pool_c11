# 设置cmake的最低版本和项目名称
cmake_minimum_required(VERSION 3.0)
project(highPerfThreadPool
        VERSION 1.0
        LANGUAGES CXX)

# 生成debug版本，可以进行gdb调试
set(CMAKE_BUILD_TYPE "Debug")

#设置c++标准版本
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 设置项目可执行文件输出的路径
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
# 设置项目库文件输出的路径
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)

# 设置项目编译头文件搜索路径 -I   todo 可能存在命名空间污染的问题，用target_include_xxx 来试试 参考pika
include_directories(${PROJECT_SOURCE_DIR}/src/version_1_inception/include)
include_directories(${PROJECT_SOURCE_DIR}/src/version_2_high_performace/include)
# 设置项目库文件搜索路径 -L
link_directories(${PROJECT_SOURCE_DIR}/lib)

# src包含了所有源代码
add_subdirectory(src)
add_subdirectory(tests)

#add_subdirectory(example)

#拉取nanoFetch
include(FetchContent)

FetchContent_Declare(
        nanobench
        GIT_REPOSITORY https://github.com/martinus/nanobench.git
        GIT_TAG v4.1.0
        GIT_SHALLOW TRUE)

FetchContent_MakeAvailable(nanobench)
#拉取nanoFetch end

add_executable(performanceTest tests/performance.cpp)
target_link_libraries(performanceTest PRIVATE version_1_inception version_2_high_performace nanobench)


