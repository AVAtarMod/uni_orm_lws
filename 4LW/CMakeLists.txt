cmake_minimum_required(VERSION 3.11)
project(ORM_4LW)

if (NOT "$ENV{CMAKE_DEFAULT_INCLUDE_PATH}" STREQUAL "")
    set(CMAKE_DEFAULT_INCLUDE_PATH "$ENV{CMAKE_DEFAULT_INCLUDE_PATH}" CACHE INTERNAL "Path for external projects and dependencies")
else()
   message(FATAL_ERROR "Cannot find exernal projects path. Set CMAKE_DEFAULT_INCLUDE_PATH environment variable")
   return()
endif()

set (CMAKE_CXX_STANDARD 17)
set(RELEASE_FLAGS "-O3 -DNDEBUG")
set(DEBUG_FLAGS "-g -DDEBUG")

# Export no symbols by default (if the compiler supports it).
# This makes e.g. GCC's "visibility behavior" consistent with MSVC's. 
# On Windows/MSVC this is a noop.
set(CMAKE_C_VISIBILITY_PRESET hidden)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)

set(CMAKE_CXX_FLAGS_RELEASE ${RELEASE_FLAGS})
set(CMAKE_C_FLAGS_RELEASE ${RELEASE_FLAGS})

set(CMAKE_CXX_FLAGS_DEBUG ${DEBUG_FLAGS})
set(CMAKE_C_FLAGS_DEBUG ${DEBUG_FLAGS})

# Always include the source and build directories in the include path.
set(CMAKE_INCLUDE_CURRENT_DIR ON)

file(TO_NATIVE_PATH ${SOURCE_DIR}/files assetsdir)
add_compile_definitions(OUTPUT_DIRECTORY=${assetsdir})
# create the executable
add_executable(main 
   main.cpp function.cpp function.hpp)

# Copy resources to build dir
file(GLOB_RECURSE data files/*)
foreach(file ${data})
   configure_file(${file} . COPYONLY)
endforeach()

add_subdirectory(${CMAKE_DEFAULT_INCLUDE_PATH}/lib_simplex_io lib_simplex_io)
target_include_directories(main PUBLIC ${CMAKE_DEFAULT_INCLUDE_PATH}/lib_simplex_io/include)
target_link_libraries(main lib_simplex_io)
