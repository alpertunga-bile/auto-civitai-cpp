cmake_minimum_required(VERSION 3.5.0)
project(auto-civitai-cpp VERSION 0.1.0 LANGUAGES C CXX)

include(CTest)
enable_testing()

# ------------------------------------------------------------------------------------
# -- GLOBAL DEFINITIONS
set(JSON_BuildTests OFF CACHE INTERNAL "")

include(setup.cmake)

set_debug()

set_cxx(20)
enable_avx2()
enable_iwyu()

set(PXD_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/sources)

set(PXD_HEADER_FILES
    ${PXD_SOURCE_DIR}/dataset.hpp
    ${PXD_SOURCE_DIR}/civitai.hpp

    third-party/json/single_include/nlohmann/json.hpp
    third-party/fmt/include/fmt/core.h
    third-party/fmt/include/fmt/os.h
)

set(PXD_SOURCE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
    ${PXD_SOURCE_DIR}/dataset.cpp
    ${PXD_SOURCE_DIR}/civitai.cpp

    ${PXD_HEADER_FILES}
)

include_directories(
    third-party/json/single_include/nlohmann
    third-party/fmt/include/fmt 
)

set(COMMON_STD_HEADERS
    <string>
    <unordered_set>
    <unordered_map>
    <regex>
    <unistd.h>
    <chrono>
    <filesystem>
    <fstream>
)

add_subdirectory(third-party/json)
add_subdirectory(third-party/fmt)

find_package(Arrow)
find_package(Parquet)
find_package(cpr)


set(LIBS_TO_LINK
    Arrow::arrow_shared
    Parquet::parquet_shared
    cpr::cpr
    nlohmann_json::nlohmann_json
    fmt::fmt
)

# ------------------------------------------------------------------------------------------------------
# -- Project Static Library 

add_executable(${PROJECT_NAME} ${PXD_SOURCE_FILES})

target_link_libraries(${PROJECT_NAME} ${LIBS_TO_LINK})

target_precompile_headers(
    ${PROJECT_NAME} PRIVATE
    ${COMMON_STD_HEADERS}
    ${PXD_HEADER_FILES}
)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)