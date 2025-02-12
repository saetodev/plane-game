cmake_minimum_required(VERSION 3.30)

project(plane)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS on)

file(GLOB_RECURSE SRC_FILES "src/*.cpp")

add_executable(${PROJECT_NAME} ${SRC_FILES})

if(WIN32)
    target_include_directories(${PROJECT_NAME} PUBLIC "vendor/sdl/include")
    target_link_directories(${PROJECT_NAME} PUBLIC "vendor/sdl/lib/x64")
    target_link_libraries(${PROJECT_NAME} "SDL2main" "SDL2")
elseif(APPLE)
    target_include_directories(${PROJECT_NAME} PUBLIC "/opt/homebrew/include")
    target_link_directories(${PROJECT_NAME} PUBLIC "/opt/homebrew/lib")
    target_link_libraries(${PROJECT_NAME} "SDL2")
endif()
