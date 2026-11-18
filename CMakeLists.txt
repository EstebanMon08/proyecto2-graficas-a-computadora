cmake_minimum_required(VERSION 3.14)
project(diorama CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

find_package(raylib 5.0 QUIET)
if(NOT raylib_FOUND)
  include(FetchContent)
  set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
  FetchContent_Declare(raylib GIT_REPOSITORY https://github.com/raysan5/raylib.git GIT_TAG 5.5)
  FetchContent_MakeAvailable(raylib)
endif()

add_executable(diorama src/main.cpp)
target_include_directories(diorama PRIVATE src)
target_link_libraries(diorama PRIVATE raylib)

find_package(Threads REQUIRED)
target_link_libraries(diorama PRIVATE Threads::Threads)

add_custom_command(TARGET diorama POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/assets $<TARGET_FILE_DIR:diorama>/assets)
