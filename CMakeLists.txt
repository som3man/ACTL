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

target_sources(ACTL
    PUBLIC
        FILE_SET CXX_MODULES FILES
            ACTL-Sort.ixx
            ACTL-Map.ixx
            ACTL-List.ixx
            ACTL-Hash.ixx
            ACTL-Utility.ixx
            ACTL-Array.ixx
            ACTL-String.ixx
            ACTL-Symbol.ixx
            ACTL-Allocation.ixx
            ACTL-Defines.ixx
            ACTL-Option.ixx
            ACTL-Delegate.ixx
            ACTL.ixx
)