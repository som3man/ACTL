cmake_minimum_required(VERSION 3.28)

project("ACTL_Test")

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS OFF)

add_subdirectory(
    "../"
    "${CMAKE_CURRENT_BINARY_DIR}/ACTL"
)

link_libraries(ACTL)

add_executable(Array "Array.cpp")

add_executable(Option "Option.cpp")

add_executable(String "String.cpp")

add_executable(Symbol "Symbol.cpp")

add_executable(Delegate "Delegate.cpp")