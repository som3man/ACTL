cmake_minimum_required(VERSION 3.28)

project("ACTL")

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(ACTL)

target_compile_definitions(ACTL INTERFACE
    ACTL_INCLUDED
)

target_compile_features(ACTL PUBLIC cxx_std_20)

target_sources(ACTL PUBLIC FILE_SET CXX_MODULES FILES
    src/ACTL-Sort.cppm
    src/ACTL-Map.cppm
    src/ACTL-List.cppm
    src/ACTL-Hash.cppm
    src/ACTL-Utility.cppm
    src/ACTL-Array.cppm
    src/ACTL-String.cppm
    src/ACTL-Symbol.cppm
    src/ACTL-Allocation.cppm
    src/ACTL-Defines.cppm
    src/ACTL-Option.cppm
    src/ACTL-Delegate.cppm
    src/ACTL-Heap.cppm
    src/ACTL.cppm
)