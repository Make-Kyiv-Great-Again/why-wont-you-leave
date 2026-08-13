cmake_minimum_required(VERSION 3.14)
project(RaylibGame C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Ensure CMake can find system frameworks on macOS
if(APPLE)
    set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64")
endif()

if(EMSCRIPTEN)
    set(PLATFORM Web)
    set(GRAPHICS GRAPHICS_API_OPENGL_ES2)
    add_compile_definitions(PLATFORM_WEB)
endif()

# Fetch raylib
include(FetchContent)
FetchContent_Declare(
    raylib
    GIT_REPOSITORY https://github.com/raysan5/raylib.git
    GIT_TAG master 
)
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(BUILD_GAMES OFF CACHE BOOL "" FORCE)
set(CUSTOMIZE_BUILD ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(raylib)

# Fetch raylib-cpp (Header only wrapper)
FetchContent_Declare(
    raylib_cpp
    GIT_REPOSITORY https://github.com/RobLoach/raylib-cpp.git
    GIT_TAG master
)
FetchContent_MakeAvailable(raylib_cpp)

file(GLOB_RECURSE SOURCES "src/*.cpp")
file(GLOB_RECURSE HEADERS "src/*.hpp")

add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

if(EMSCRIPTEN)
    set_target_properties(${PROJECT_NAME} PROPERTIES SUFFIX ".html")
    set_property(TARGET ${PROJECT_NAME} APPEND PROPERTY LINK_DEPENDS "${CMAKE_SOURCE_DIR}/web/shell.html")
    target_link_options(${PROJECT_NAME} PRIVATE 
        "-s" "USE_GLFW=3"
        "--preload-file" "${CMAKE_SOURCE_DIR}/assets@assets"
        "--shell-file" "${CMAKE_SOURCE_DIR}/web/shell.html"
    )
endif()

# Link libraries
target_link_libraries(${PROJECT_NAME} PRIVATE raylib)
target_include_directories(${PROJECT_NAME} PRIVATE ${raylib_cpp_SOURCE_DIR}/include src)

# Copy assets directory to the output directory so the executable can find them
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/assets $<TARGET_FILE_DIR:${PROJECT_NAME}>/assets
)
