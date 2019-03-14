cmake_minimum_required(VERSION 3.13.3)
project(lutherie)

set(LUA_LIB_DIR /usr/local/lib CACHE PATH "")
set(LUA_INCLUDE_DIR /usr/local/include CACHE PATH "")

set(GLFW_LIB_DIR /usr/local/lib CACHE PATH "")
set(GLFW_INCLUDE_DIR /usr/local/include CACHE PATH "")
	
set(VULKAN_LIB_DIR $ENV{HOME}/vulkansdk-macos-1.1.92.1/macOS/lib CACHE PATH "")
set(VULKAN_INCLUDE_DIR $ENV{HOME}/vulkansdk-macos-1.1.92.1/macOS/include CACHE PATH "")

## ECS Library 

add_library(ECS SHARED 
    ECS/src/ecs.cpp
)

add_custom_command(TARGET ECS PRE_BUILD COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/libs)

target_include_directories(ECS PUBLIC ECS/include)

set_target_properties(ECS PROPERTIES 
    CXX_STANDARD 17
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/libs
)

## ECSlua Library

add_library(ECSlua SHARED
    ECSlua/src/ECSlua.cpp
)

target_include_directories(ECSlua PUBLIC ECSlua/include ${LUA_INCLUDE_DIR})
target_link_directories(ECSlua PUBLIC ${CMAKE_BINARY_DIR}/libs ${LUA_LIB_DIR})
target_link_libraries(ECSlua luajit ECS)

set(LINK_LIBS glfw vulkan ECSlua)
set(SOURCES src/lutherie.cpp src/main.cpp)

set_target_properties(ECSlua PROPERTIES 
    CXX_STANDARD 17
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/libs
)

## Executable

add_executable(lutherie ${SOURCES})

target_link_directories(lutherie PUBLIC ${CMAKE_BINARY_DIR}/libs ${GLFW_LIB_DIR} ${VULKAN_LIB_DIR})

target_include_directories(lutherie PUBLIC ECSlua/include include ${GLFW_INCLUDE_DIR} ${VULKAN_INCLUDE_DIR})

set_target_properties(lutherie PROPERTIES 
	CXX_STANDARD 17
)
if(APPLE)
target_link_options(lutherie PUBLIC "-pagezero_size 10000 -image_base 100000000")
endif()
target_link_libraries(lutherie ${LINK_LIBS})

add_custom_command(TARGET ECS PRE_BUILD COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/libs/lua)

set(LIB_LUA_DIR libs/lua)

function(add_lua_file LUA_NAME)
    add_custom_command(OUTPUT ${LIB_LUA_DIR}/${LUA_NAME}.raw 
        COMMAND luajit ARGS -b ${CMAKE_SOURCE_DIR}/scripts/${LUA_NAME}.lua ${CMAKE_BINARY_DIR}/${LIB_LUA_DIR}/${LUA_NAME}.raw
        DEPENDS ${CMAKE_SOURCE_DIR}/scripts/${LUA_NAME}.lua)
    add_custom_target(${LUA_NAME}.lua ALL DEPENDS ${CMAKE_BINARY_DIR}/${LIB_LUA_DIR}/${LUA_NAME}.raw)
endfunction()

add_lua_file(lutherie)
