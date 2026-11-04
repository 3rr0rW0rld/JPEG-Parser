cmake_minimum_required(VERSION 3.23)


set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(jpeg_proj)
add_executable(jpeg_proj)

file(COPY images/
    DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/images/)

target_sources(jpeg_proj
PRIVATE
    main.cpp
    ByteFileReader.cpp
    JPEGParser.cpp

PUBLIC
    FILE_SET HEADERS
    FILES
        ByteFileReader.h
        JPEGParser.h
)