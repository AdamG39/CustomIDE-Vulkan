cmake_minimum_required(VERSION 3.10)
project(CustomIDE)

set(CMAKE_CXX_STANDARD 17)

# Vulkan
find_package(Vulkan REQUIRED)

# GLFW
add_subdirectory(externals/glfw)
include_directories(externals/glfw/include)

file(GLOB SRC_FILES "src/*.cpp")

add_executable(CustomIDE ${SRC_FILES})

target_include_directories(CustomIDE PRIVATE ${Vulkan_INCLUDE_DIRS})
target_link_libraries(CustomIDE PRIVATE ${Vulkan_LIBRARIES})
target_link_libraries(CustomIDE PRIVATE glfw Vulkan::Vulkan)
