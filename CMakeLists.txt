cmake_minimum_required(VERSION 3.27)
project(MediaFormatParser)

set(CMAKE_CXX_STANDARD 17)

find_path(MPG123_INCLUDE_DIR mpg123.h
        PATHS /opt/homebrew/include /usr/local/include
)
find_library(MPG123_LIBRARY mpg123
        PATHS /opt/homebrew/lib /usr/local/lib
)

file(GLOB_RECURSE SRCS
    ${CMAKE_SOURCE_DIR}/src/*.cpp
    ${CMAKE_SOURCE_DIR}/src/*.cc
)

if (NOT MPG123_INCLUDE_DIR OR NOT MPG123_LIBRARY)
    message(FATAL_ERROR "mpg123 not found. Please install with: brew install mpg123")
endif()

include_directories(${MPG123_INCLUDE_DIR})
link_libraries(${MPG123_LIBRARY})

add_executable(MediaFormatParser ${SRCS})

target_link_libraries(MediaFormatParser ${MPG123_LIBRARY})
