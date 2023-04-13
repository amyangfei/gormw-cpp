cmake_minimum_required(VERSION 3.14)
project(gormw_cpp_example CXX)
set(CMAKE_CXX_STANDARD 14)

include(FetchContent)
FetchContent_Declare(
  gor
  GIT_REPOSITORY    https://github.com/amyangfei/gormw-cpp.git
  GIT_TAG           main
)
FetchContent_MakeAvailable(gor)

add_executable(example example.cpp)
target_include_directories(example PRIVATE ${gor_SOURCE_DIR}/src/include)
target_link_libraries(example PRIVATE gor z)