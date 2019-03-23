cmake_minimum_required(VERSION 3.13.3)

project(lutherie)

set(CMAKE_BUILD_TYPE Debug)

if(APPLE)
    find_library(COCOA_LIB Cocoa)
    if(NOT COCOA_LIB)
        message(FATAL_ERROR "Can't find Cocoa!")
    endif()
    find_library(IOKIT_LIB IOKit)
    if(NOT IOKIT_LIB)
        message(FATAL_ERROR "Can't find IOKit!")
    endif()
    find_library(COREFOUNDATION_LIB CoreFoundation)
    if(NOT COREFOUNDATION_LIB)
        message(FATAL_ERROR "Can't find CoreFoundation!")
    endif()
    find_library(COREVIDEO_LIB CoreVideo)
    if(NOT COREVIDEO_LIB)
        message(FATAL_ERROR "Can't find CoreVideo!")
    endif()
endif()

# find_package(PkgConfig)

include(GenerateExportHeader)

# if(PkgConfig_FOUND)
#    pkg_check_modules(GLFW glfw3)
#    pkg_check_modules(LUA luajit)   
# endif()

find_package(Vulkan)

# if(NOT LUA_FOUND)
#    set(LUA_LIBRARIES luajit CACHE STRING "")
#    set(LUA_LIBRARY_DIRS /usr/local/lib CACHE PATH "")
#    set(LUA_INCLUDE_DIRS /usr/local/include CACHE PATH "")
# endif()

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/libs/static)

##################
##              ##
##    ECS       ##
##              ##
##################

add_library(ECS STATIC
    ECS/src/ECS.cpp
)

generate_export_header(ECS
	STATIC_DEFINE SHARED_EXPORTS_BUILT_AS_STATIC
)

add_custom_command(TARGET ECS PRE_BUILD COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}/libs)

target_include_directories(ECS PUBLIC ${CMAKE_BINARY_DIR} ECS/include)

set_target_properties(ECS PROPERTIES 
    CXX_STANDARD 17
)

##################
##              ##
##    LuaJIT    ##
##              ##
##################


if(APPLE)
    if(NOT ENV{MACOSX_DEPLOYMENT_TARGET})
        set(ENV{MACOSX_DEPLOYMENT_TARGET} 10.14)
        message(WARNING "ENV{MACOSX_DEPLOYMENT_TARGET} wasn't set, $ENV{MACOSX_DEPLOYMENT_TARGET} assumed")
    endif()
endif()

if(WIN32 OR WIN64)
    set(MAKE_COMMAND ${CMAKE_SOURCE_DIR}/libs/luajit/src/msvcbuild)
    set(LUAJIT_EXE luajit.exe)
else()
    set(MAKE_COMMAND make -C ${CMAKE_SOURCE_DIR}/libs/luajit)
    set(LUAJIT_EXE luajit)
endif()

file(GLOB LUAJIT_SRC CONFIGURE_DEPENDS ${CMAKE_SOURCE_DIR}/libs/luajit/src/*.c)

set(LUAJIT_PREFIX PREFIX=${CMAKE_BINARY_DIR}/luajit)

if(WIN32 OR WIN64)

else()
    if(APPLE)
        set(ENVIRONMENT_COMMAND export MACOSX_DEPLOYMENT_TARGET=$ENV{MACOSX_DEPLOYMENT_TARGET} &&)
    endif()
    add_custom_command(OUTPUT ${CMAKE_BINARY_DIR}/luajit/bin/${LUAJIT_EXE}
        COMMAND ${ENVIRONMENT_COMMAND}
        ${MAKE_COMMAND} ${LUAJIT_PREFIX}
        && ${MAKE_COMMAND} install ${LUAJIT_PREFIX}
        DEPENDS ${LUAJIT_SRC}
    )
endif()

add_custom_target(luaJIT ALL DEPENDS ${CMAKE_BINARY_DIR}/luajit/bin/${LUAJIT_EXE})

# add_custom_command(OUTPUT ${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}/bin/${LUAJIT_EXE}
#    COMMAND ${CMAKE_COMMAND} ARGS -E copy ${CMAKE_SOURCE_DIR}/libs/luajit/src/${LUAJIT_EXE} ${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}/bin/
#    DEPENDS ${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}/bin/${LUAJIT_EXE}
# )



##################
##              ##
##    ECSlua    ##
##              ##
##################

add_library(ECSlua STATIC
    ECSlua/src/ECSlua.cpp
)

add_dependencies(ECSlua luaJIT)

generate_export_header(ECSlua
	STATIC_DEFINE SHARED_EXPORTS_BUILT_AS_STATIC
)

target_include_directories(ECSlua PUBLIC ECSlua/include ${CMAKE_SOURCE_DIR}/libs/luajit/src)
target_link_directories(ECSlua PUBLIC ${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}/libs ${CMAKE_BINARY_DIR} ${LUA_LIBRARY_DIRS})
target_link_libraries(ECSlua ${LUA_LIBRARIES} ECS)

set_target_properties(ECSlua PROPERTIES 
    CXX_STANDARD 17
)

##################
##              ##
##    GLFW      ##
##              ##
##################

# if(APPLE)
    #message(VULKAN ${VULKAN_INCLUDE_DIR})
    #set(VULKAN_STATIC_LIBRARY ${VULKAN_INCLUDE_DIR}/../../MoltenVK/macOS/static/libMoltenVK.a)
# endif()

set(GLFW_BUILD_DOCS off)
add_subdirectory(libs/glfw ${CMAKE_BINARY_DIR}/glfw)

add_custom_target(staticLibs)
add_dependencies(staticLibs glfw)

##################
##              ##
##  Lutherie    ##
##              ##
##################

set(SOURCES src/lutherie.cpp src/main.cpp)

add_executable(lutherie ${SOURCES})
add_dependencies(lutherie staticLibs)

set(LINK_LIBS ${GLFW_LIBRARIES} ECSlua glfw)

target_link_directories(lutherie PUBLIC ${CMAKE_SOURCE_DIR}/libs/static)

target_include_directories(lutherie PUBLIC ECSlua/include include ${CMAKE_SOURCE_DIR}/libs/glfw/include ${CMAKE_BINARY_DIR}/luajit/include ${Vulkan_INCLUDE_DIRS})

set_target_properties(lutherie PROPERTIES 
	CXX_STANDARD 17
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}
)

if(WIN32)
    add_definitions(-DNODEFAULTLIB:library -D_CRT_SECURE_NO_WARNINGS)
endif()

if(APPLE)
    add_definitions(-DLUTHERIE_MAC)
    target_link_options(lutherie PUBLIC "-pagezero_size 10000 -image_base 100000000")
    target_link_libraries(lutherie ${COCOA_LIB} ${IOKIT_LIB} ${COREFOUNDATION_LIB} ${COREVIDEO_LIB})
endif()

if(WIN32 OR WIN64)
    set(LUAJIT_LIB ${CMAKE_BINARY_DIR}/luajit/lib/lua51.lib)
else()
    set(LUAJIT_LIB ${CMAKE_BINARY_DIR}/luajit/lib/libluajit-5.1.a)
endif()

target_link_libraries(lutherie ${LINK_LIBS} ${LUAJIT_LIB})

add_custom_command(TARGET ECS PRE_BUILD COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}/libs/lua)

set(LIB_LUA_DIR libs/lua)

function(add_lua_file LUA_NAME)
    add_custom_command(OUTPUT ${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}/${LIB_LUA_DIR}/${LUA_NAME}.raw 
        COMMAND luajit ARGS -b ${CMAKE_SOURCE_DIR}/scripts/${LUA_NAME}.lua ${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}/${LIB_LUA_DIR}/${LUA_NAME}.raw
        DEPENDS ${CMAKE_SOURCE_DIR}/scripts/${LUA_NAME}.lua)
    add_custom_target(${LUA_NAME}.lua ALL DEPENDS ${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}/${LIB_LUA_DIR}/${LUA_NAME}.raw)
    add_dependencies(${LUA_NAME}.lua lutherie)
endfunction()

add_lua_file(lutherie)
