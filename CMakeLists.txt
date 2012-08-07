project(SpellChecker)
cmake_minimum_required(VERSION 2.8)

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")

aux_source_directory(. SRC_LIST)
file(GLOB HDR_LIST "*.h")

find_package(Threads)

add_executable(${PROJECT_NAME} ${HDR_LIST} ${SRC_LIST})
target_link_libraries(${PROJECT_NAME} ${CMAKE_THREAD_LIBS_INIT})
