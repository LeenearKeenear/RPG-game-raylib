cmake_minimum_required(VERSION 3.20)
project(RPGGame VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Unity build: combine all sources into single translation unit
# This parses tileson.hpp only once instead of per .cpp file
set(CMAKE_UNITY_BUILD ON)
set(CMAKE_UNITY_BUILD_PRUNE_HEIGHT 0)

# Output binaries to build/ directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

if(MSVC)
    set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -Wno-missing-field-initializers)
endif()

# Enable ccache for faster recompilation
find_program(CCACHE_PROGRAM ccache)
if(CCACHE_PROGRAM)
    set(CMAKE_CXX_COMPILER_LAUNCHER ${CCACHE_PROGRAM})
endif()

# Automatic source file discovery
file(GLOB_RECURSE SOURCES "src/*.cpp")

# Create executable
add_executable(main ${SOURCES})

# Include directories
target_include_directories(main PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/lib/tileson>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/lib/raylib/include>
)

# Raylib library
if(WIN32)
    set(RAYLIB_PATH "${CMAKE_CURRENT_SOURCE_DIR}/lib/raylib/lib/libraylib.a")
    set(RAYLIB_DLL "${CMAKE_CURRENT_SOURCE_DIR}/lib/raylib/lib/raylib.dll")

    target_link_libraries(main PRIVATE "${RAYLIB_PATH}")
    target_link_libraries(main PRIVATE opengl32 gdi32 winmm)

    if(EXISTS "${RAYLIB_DLL}")
    add_custom_command(TARGET main POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_FILE_DIR:main>
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${RAYLIB_DLL}"
            $<TARGET_FILE_DIR:main>
        COMMENT "Copying raylib.dll to output directory"
    )
endif()
elseif(UNIX)
    if(APPLE)
        target_link_libraries(main PRIVATE -framework OpenGL -framework Cocoa -framework IOKit -framework CoreVideo)
    else()
        target_link_libraries(main PRIVATE GL m pthread dl rt)
    endif()
endif()