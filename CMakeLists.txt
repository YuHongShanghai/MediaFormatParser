cmake_minimum_required(VERSION 3.27)
project(MediaFormatParser)

set(CMAKE_CXX_STANDARD 17)

file(GLOB_RECURSE SRCS
        RELATIVE ${CMAKE_SOURCE_DIR}
        *.cpp
)

# 手动过滤掉构建目录下的文件
set(FILTERED_SRCS "")
foreach(file ${SRCS})
    if(NOT file MATCHES "^cmake-build-debug/")
        list(APPEND FILTERED_SRCS ${file})
    endif()
endforeach()

add_executable(MediaFormatParser ${FILTERED_SRCS})
