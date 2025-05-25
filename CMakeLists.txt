cmake_minimum_required(VERSION 3.27)
project(MediaFormatParser)

set(CMAKE_CXX_STANDARD 17)

file(GLOB_RECURSE SRCS
    ${CMAKE_SOURCE_DIR}/src/*.cpp
    ${CMAKE_SOURCE_DIR}/src/*.cc
)

add_executable(MediaFormatParser ${SRCS})
target_include_directories(MediaFormatParser PRIVATE ${CMAKE_SOURCE_DIR}/logger)
