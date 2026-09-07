cmake_minimum_required(VERSION 3.28)

project("ACTL")

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(ACTL)

target_compile_features(ACTL PUBLIC cxx_std_20)

target_sources(ACTL PUBLIC FILE_SET CXX_MODULES FILES
    src/Main.cppm
    src/Defines.cppm
    src/Array.cppm
    src/String.cppm
    src/Option.cppm
    src/Heap.cppm
)