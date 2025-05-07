cmake_minimum_required(VERSION 3.10)
project(ACA-BP LANGUAGES CXX)

option(DEBUG "Enable debugging symbols" OFF)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(DEBUG)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ggdb3")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0")
endif()

# Paths
set(LIB_DIR ${CMAKE_SOURCE_DIR}/lib)
set(LIBCBP_PATH ${LIB_DIR}/libcbp.a)

# Source files for main cbp executable
file(GLOB_RECURSE SOURCES
        cond_branch_predictor_interface.cc
        my_cond_branch_predictor.cc
        tage/*.h
)

# Include directories
include_directories(${CMAKE_SOURCE_DIR})
include_directories(${LIB_DIR})

# Custom target to build libcbp.a using Makefile in lib/
add_custom_command(
        OUTPUT ${LIBCBP_PATH}
        COMMAND ${CMAKE_MAKE_PROGRAM} -C ${LIB_DIR} DEBUG=$<IF:$<BOOL:${DEBUG}>,1,0>
        DEPENDS ${LIB_DIR}/Makefile
        COMMENT "Building libcbp.a in lib/"
)

add_custom_target(build_cbp_lib DEPENDS ${LIBCBP_PATH})

# Add the main executable
add_executable(cbp ${SOURCES})

# Ensure libcbp.a is built before the executable
add_dependencies(cbp build_cbp_lib)

# Link manually with the static archive and zlib
target_link_libraries(cbp PRIVATE ${LIBCBP_PATH} z)