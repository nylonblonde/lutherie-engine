cmake_minimum_required(VERSION 3.13.3)

project(lutherie)

find_package(PkgConfig)

include(GenerateExportHeader)

if(PkgConfig_FOUND)
    pkg_check_modules(GLFW glfw3)
    pkg_check_modules(LUA luajit)
    
endif()

find_package(Vulkan)
if(Vulkan_FOUND)
    set(VULKAN_LIBRARIES ${Vulkan_LIBRARIES})
    set(VULKAN_LIBRARY_DIRS ${Vulkan_LIBRARY_DIRS})
    set(VULKAN_INCLUDE_DIRS ${Vulkan_INCLUDE_DIRS})
endif()

if(NOT LUA_FOUND)
    set(LUA_LIBRARIES luajit CACHE STRING "")
    set(LUA_LIBRARY_DIRS /usr/local/lib CACHE PATH "")
    set(LUA_INCLUDE_DIRS /usr/local/include CACHE PATH "")
endif()

if(NOT GLFW_FOUND)
    set(GLFW_LIBRARIES glfw CACHE STRING "")
    set(GLFW_LIBRARY_DIRS /usr/local/lib CACHE PATH "")
    set(GLFW_INCLUDE_DIRS /usr/local/include CACHE PATH "")
endif()
	
if(NOT Vulkan_FOUND)
    set(VULKAN_LIBRARIES vulkan CACHE STRING "")
    set(VULKAN_LIBRARY_DIRS $ENV{VULKAN_SDK}/lib CACHE PATH "")
    set(VULKAN_INCLUDE_DIRS $ENV{VULKAN_SDK}/include CACHE PATH "")
endif()

## ECS Library 

add_library(ECS STATIC
    ECS/src/ECS.cpp
)

generate_export_header(ECS
	STATIC_DEFINE SHARED_EXPORTS_BUILT_AS_STATIC
)

add_custom_command(TARGET ECS PRE_BUILD COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/libs)

target_include_directories(ECS PUBLIC ${CMAKE_BINARY_DIR} ECS/include)

set_target_properties(ECS PROPERTIES 
    CXX_STANDARD 17
)

## ECSlua Library

add_library(ECSlua STATIC
    ECSlua/src/ECSlua.cpp
)

generate_export_header(ECSlua
	STATIC_DEFINE SHARED_EXPORTS_BUILT_AS_STATIC
)


target_include_directories(ECSlua PUBLIC ECSlua/include ${LUA_INCLUDE_DIRS})
target_link_directories(ECSlua PUBLIC ${CMAKE_BINARY_DIR}/libs ${CMAKE_BINARY_DIR} ${LUA_LIBRARY_DIRS})
target_link_libraries(ECSlua ${LUA_LIBRARIES} ECS)

set(LINK_LIBS ${GLFW_LIBRARIES} ${VULKAN_LIBRARIES} ECSlua)
set(SOURCES src/lutherie.cpp src/main.cpp)

set_target_properties(ECSlua PROPERTIES 
    CXX_STANDARD 17
    #LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/libs
    #ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/libs
)

## Executable

add_executable(lutherie ${SOURCES})

target_link_directories(lutherie PUBLIC ${CMAKE_BINARY_DIR}/libs ${GLFW_LIBRARY_DIRS} ${VULKAN_LIBRARY_DIRS})

target_include_directories(lutherie PUBLIC ECSlua/include include ${GLFW_INCLUDE_DIRS} ${VULKAN_INCLUDE_DIRS})

set_target_properties(lutherie PROPERTIES 
	CXX_STANDARD 17
)

if(WIN32)
    add_definitions(-DNODEFAULTLIB:library -D_CRT_SECURE_NO_WARNINGS)
endif()

if(APPLE)
    add_definitions(-DLUTHERIE_MAC)
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
    add_dependencies(${LUA_NAME}.lua lutherie)
endfunction()

add_lua_file(lutherie)
