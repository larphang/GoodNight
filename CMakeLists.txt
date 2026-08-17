cmake_minimum_required(VERSION 3.20)

project(GoodNight LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(GoodNight WIN32
    src/main.cpp
)

target_compile_definitions(GoodNight PRIVATE
    UNICODE
    _UNICODE
)

target_link_libraries(GoodNight PRIVATE
    user32
    gdi32
)