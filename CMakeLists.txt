cmake_minimum_required(VERSION 3.10)
project(Evently)

# 使用 C++17 标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_subdirectory(googletest)

# 包含 include 目录下的头文件
include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${CMAKE_SOURCE_DIR}/googletest/googlemock/include)
include_directories(${CMAKE_SOURCE_DIR}/googletest/googletest/include)

# 将 src 目录下的所有 .cpp 文件加入 SOURCES
file(GLOB SOURCES ${CMAKE_SOURCE_DIR}/src/*.cpp)

# 添加可执行文件
add_executable(Evently ${SOURCES})

target_link_libraries(Evently pthread gtest gtest_main)
