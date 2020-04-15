cmake_minimum_required(VERSION 3.14)
project(SmartMap)


set(CMAKE_CXX_STANDARD 17)


add_executable(SmartMap
    include/SmartMap.hpp
    include/SmartMap.inl
    src/SmartMap.cpp
    src/main.cpp
)

target_include_directories(SmartMap
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include/
)
