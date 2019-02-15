cmake_minimum_required(VERSION 3.13.3)
project(Lutherie)


set(GLFW_LIB_DIR /usr/local/lib CACHE PATH "")
set(GLFW_INCLUDE_DIR /usr/local/include CACHE PATH "")
	
set(VULKAN_LIB_DIR $ENV{HOME}/vulkansdk-macos-1.1.92.1/macOS/lib CACHE PATH "")
set(VULKAN_INCLUDE_DIR $ENV{HOME}/vulkansdk-macos-1.1.92.1/macOS/include CACHE PATH "")

link_directories(${GLFW_LIB_DIR} ${VULKAN_LIB_DIR})


set(LINK_LIBS glfw vulkan)

include_directories(include ${GLFW_INCLUDE_DIR} ${VULKAN_INCLUDE_DIR})

file(GLOB SOURCES src/*.cpp)

add_executable(Lutherie ${SOURCES})

set_target_properties(Lutherie PROPERTIES 
	CXX_STANDARD 17
	)

target_link_libraries(Lutherie ${LINK_LIBS})
