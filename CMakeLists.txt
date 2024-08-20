cmake_minimum_required(VERSION 3.20)
project(efestftp VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(${PROJECT_NAME} src/main.cc)
target_include_directories(${PROJECT_NAME} PUBLIC include)

add_subdirectory(argparse)
add_subdirectory(json)
add_subdirectory(spdlog)

if(BUILD_TESTING)
    message(STATUS "Building tests is enable")
    add_subdirectory(googletest)
    add_subdirectory(tests)
endif()

target_link_libraries(${PROJECT_NAME} PRIVATE 
    argparse
    nlohmann_json
    spdlog
)
