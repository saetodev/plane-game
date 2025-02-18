cmake_minimum_required(VERSION 3.30)

project(plane)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS on)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")

file(GLOB_RECURSE SRC_FILES "src/*.cpp")
file(GLOB IMGUI_SRC_FILES "vendor/imgui/*.cpp")

add_subdirectory("vendor/SDL" EXCLUDE_FROM_ALL)

add_executable(${PROJECT_NAME} ${SRC_FILES} ${IMGUI_SRC_FILES} "vendor/glad/src/glad.c" "vendor/imgui/backends/imgui_impl_opengl3.cpp" "vendor/imgui/backends/imgui_impl_sdl2.cpp")

target_include_directories(${PROJECT_NAME} PUBLIC "src" "vendor/SDL/include" "vendor/glad/include" "vendor/stb" "vendor/imgui" "vendor/imgui/backends")

if(WIN32)
    target_link_directories(${PROJECT_NAME} PUBLIC "vendor/sdl/lib/x64")
    target_link_libraries(${PROJECT_NAME} "SDL2main" "SDL2" "opengl32")
elseif(APPLE)
    target_include_directories(${PROJECT_NAME} PUBLIC "vendor/angle/include")
    target_link_directories(${PROJECT_NAME} PUBLIC "vendor/angle/out/Release")
    target_link_libraries(${PROJECT_NAME} "SDL2" "EGL" "GLESv2")
endif()
