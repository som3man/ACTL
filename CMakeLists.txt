cmake_minimum_required (VERSION 3.12)

project ("ACTL")

add_executable (ACTL "include/ACTL/ACTL.hpp" "ACTL.cpp")

set_property (TARGET ACTL PROPERTY CXX_STANDARD 20)
set_property (TARGET ACTL PROPERTY CXX_STANDARD_REQUIRED ON)
set_property (TARGET ACTL PROPERTY CXX_EXTENSIONS OFF)
