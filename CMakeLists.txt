cmake_minimum_required(VERSION 3.23)

project(jpeg_proj)
add_executable(jpeg_proj)

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(COPY images/
    DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/images/)

target_sources(jpeg_proj
PRIVATE
    main.cpp
    JPEGLoader.cpp

PUBLIC
    FILE_SET HEADERS
    FILES
        JPEGLoader.h
        huffman.h

)