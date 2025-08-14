cmake_minimum_required(VERSION 3.20)

project(
    "footprints"
    VERSION 1.0
    DESCRIPTION "A personal diary web by C++"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# file(GLOB_RECURSE MAIN_SOURCE_FILES
#     ${CMAKE_SOURCE_DIR}/src/*.cpp
#     ${CMAKE_SOURCE_DIR}/src/*.h
# )

set(MAIN_SOURCE_FILES
    src/main.cpp
    src/comm/log.cpp
)

file(COPY ${CMAKE_SOURCE_DIR}/src/html DESTINATION ${CMAKE_SOURCE_DIR}/build)

add_executable(${PROJECT_NAME} ${MAIN_SOURCE_FILES})

target_compile_definitions(${PROJECT_NAME}
    PRIVATE
        LOG_FILES_PATH="${CMAKE_SOURCE_DIR}/logs/"
)
target_include_directories(${PROJECT_NAME}
    PRIVATE
        ${CMAKE_SOURCE_DIR}/src
)
