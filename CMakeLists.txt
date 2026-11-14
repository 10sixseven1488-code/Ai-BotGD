cmake_minimum_required(VERSION 3.21)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(AIDriver VERSION 0.1.0)

file(GLOB SOURCES src/*.cpp)
add_library(${PROJECT_NAME} SHARED ${SOURCES})

if (NOT DEFINED ENV{GEODE_SDK})
    message(FATAL_ERROR "Переменная окружения GEODE_SDK не задана. Установи Geode CLI: https://docs.geode-sdk.org/getting-started/install-tools")
else()
    message(STATUS "Found Geode SDK: $ENV{GEODE_SDK}")
endif()

include($ENV{GEODE_SDK}/loader/include/link.cmake)

add_subdirectory($ENV{GEODE_SDK} ${CMAKE_CURRENT_BINARY_DIR}/geode)

setup_geode_mod(${PROJECT_NAME})
