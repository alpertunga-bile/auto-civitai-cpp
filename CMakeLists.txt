cmake_minimum_required(VERSION 3.5.0)
project(pxd-stl VERSION 0.1.0 LANGUAGES C CXX)

include(CTest)
enable_testing()

# ------------------------------------------------------------------------------------
# -- GLOBAL DEFINITIONS

include(setup.cmake)

set_debug()

set_cxx(20)
enable_avx2()
enable_iwyu()

set(PXD_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/sources)

set(PXD_HEADER_FILES
)

set(PXD__SOURCE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp

    ${PXD_HEADER_FILES}
)

include_directories(
)

set(COMMON_STD_HEADERS
    <stdlib.h>
    <cstdint>
    <vector>
    <algorithm>
    <cassert>
    <utility>
    <cstring>

    # Logger is using these headers
    <filesystem>
    <fstream>
)

set(LIBS_TO_LINK
)

# ------------------------------------------------------------------------------------------------------
# -- Project Static Library 

add_executable(${PROJECT_NAME} ${PXD__SOURCE_FILES})

target_link_libraries(${PROJECT_NAME} ${LIBS_TO_LINK})

target_precompile_headers(
    ${PROJECT_NAME} PRIVATE
    ${COMMON_STD_HEADERS}
    ${PXD_HEADER_FILES}
)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)