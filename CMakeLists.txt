cmake_minimum_required(VERSION 3.13.3)
project(Lutherie)

set(LUA_LIB_DIR /usr/local/lib CACHE PATH "")
set(LUA_INCLUDE_DIR /usr/local/include CACHE PATH "")

set(GLFW_LIB_DIR /usr/local/lib CACHE PATH "")
set(GLFW_INCLUDE_DIR /usr/local/include CACHE PATH "")
	
set(VULKAN_LIB_DIR $ENV{HOME}/vulkansdk-macos-1.1.92.1/macOS/lib CACHE PATH "")
set(VULKAN_INCLUDE_DIR $ENV{HOME}/vulkansdk-macos-1.1.92.1/macOS/include CACHE PATH "")

set(LINK_LIBS glfw vulkan luajit ECS)
set(SOURCES src/lutherie.cpp src/main.cpp)

add_executable(Lutherie ${SOURCES})

target_link_directories(Lutherie PUBLIC ECS/build ${LUA_LIB_DIR} ${GLFW_LIB_DIR} ${VULKAN_LIB_DIR})

target_include_directories(Lutherie PUBLIC ECS/include include ${LUA_INCLUDE_DIR} ${GLFW_INCLUDE_DIR} ${VULKAN_INCLUDE_DIR})

set_target_properties(Lutherie PROPERTIES 
	CXX_STANDARD 17
	)

target_link_options(Lutherie PUBLIC "-pagezero_size 10000 -image_base 100000000")

target_link_libraries(Lutherie ${LINK_LIBS})
