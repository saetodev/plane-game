cmake_minimum_required(VERSION 3.30)

project(plane)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS on)

file(GLOB_RECURSE SRC_FILES "src/*.cpp")

include_directories(src)

add_executable(${PROJECT_NAME} ${SRC_FILES} "vendor/glad/src/glad.c")

if(WIN32)
    target_include_directories(${PROJECT_NAME} PUBLIC "vendor/sdl/include" "vendor/glad/include" "vendor/stb")
    target_link_directories(${PROJECT_NAME} PUBLIC "vendor/sdl/lib/x64")
    target_link_libraries(${PROJECT_NAME} "SDL2main" "SDL2" "SDL2_image" "opengl32")
elseif(APPLE)
    target_include_directories(${PROJECT_NAME} PUBLIC "/opt/homebrew/include" "vendor/glad/include" "vendor/stb" "vendor/angle/include")
    target_link_directories(${PROJECT_NAME} PUBLIC "/opt/homebrew/lib" "vendor/angle/out/Release")
    target_link_libraries(${PROJECT_NAME} "SDL2" "SDL2_image" "EGL" "GLESv2")
endif()
