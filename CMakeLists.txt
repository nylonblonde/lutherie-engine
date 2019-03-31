cmake_minimum_required(VERSION 3.13.3)

project(lutherie)

set(CMAKE_BUILD_TYPE Release CACHE STRING "")

if(WIN32 OR WIN64)
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MD")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MDd")
endif()

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

find_package(Vulkan)

if(APPLE)
#   find_library(Vulkan_FRAMEWORK
#       NAMES vulkan
#       PATHS $ENV{VULKAN_SDK}/Frameworks
#   )

    set(RUNTIME_OUTPUT ${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}/Lutherie.app/Contents/MacOS)
else()
    set(RUNTIME_OUTPUT ${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE})
endif()

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${RUNTIME_OUTPUT}/lib/static)

##################
##              ##
##    ECS       ##
##              ##
##################

add_library(ECS STATIC
    ECS/src/ECS.cpp
)

target_include_directories(ECS PUBLIC ${CMAKE_BINARY_DIR} ECS/include)

set_target_properties(ECS PROPERTIES 
    CXX_STANDARD 17
)

add_custom_command(TARGET ECS PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/ECS/include ${RUNTIME_OUTPUT}/include
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

if(WIN32 OR WIN64) #win32 or win64 platforms
    set(MAKE_COMMAND ${CMAKE_SOURCE_DIR}/lib/luajit/src/msvcbuild.bat static)
    set(LUAJIT_EXE luajit.exe)
	set(OUTPUT_FILES  ${RUNTIME_OUTPUT}/bin/${LUAJIT_EXE})

	set(OUTPUT_COMMAND 
		setlocal && 
		cd ${CMAKE_SOURCE_DIR}/lib/luajit/src/ &&
		${MAKE_COMMAND} && 
		endlocal &&
		echo endlocal &&
		${CMAKE_COMMAND} -E make_directory ${RUNTIME_OUTPUT}/include/luajit-2.0 &&
		echo "made directory include" &&
		${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/lib/luajit/src/lua.hpp ${RUNTIME_OUTPUT}/include/luajit-2.0/ &&
		${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/lib/luajit/src/lua.h ${RUNTIME_OUTPUT}/include/luajit-2.0/ &&
		${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/lib/luajit/src/lauxlib.h ${RUNTIME_OUTPUT}/include/luajit-2.0/ &&
		${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/lib/luajit/src/lualib.h ${RUNTIME_OUTPUT}/include/luajit-2.0/ &&
		${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/lib/luajit/src/luajit.h ${RUNTIME_OUTPUT}/include/luajit-2.0/ &&
		${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/lib/luajit/src/luaconf.h ${RUNTIME_OUTPUT}/include/luajit-2.0/ &&
		${CMAKE_COMMAND} -E make_directory ${RUNTIME_OUTPUT}/bin/lua/jit &&
		echo "made directory bin/lua/jit" &&
		${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/lib/luajit/src/jit ${RUNTIME_OUTPUT}/bin/lua/jit &&
		echo "copied src/jit to lua/jit" &&
		# ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/luajit/lib &&
		${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/lib/luajit/src/${LUAJIT_EXE} ${RUNTIME_OUTPUT}/bin/ &&
		echo "copied executable" &&
		${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/lib/luajit/src/lua51.lib ${RUNTIME_OUTPUT}/lib && 
		# ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/lib/luajit/src/lua51.dll ${RUNTIME_OUTPUT}/bin && 
		echo "copied libraries"
	)

else() #unix/apple platforms
    set(MAKE_COMMAND make -C ${CMAKE_SOURCE_DIR}/lib/luajit)
    set(LUAJIT_EXE luajit)
	set(OUTPUT_FILES ${CMAKE_BINARY_DIR}/luajit/bin/${LUAJIT_EXE})
    set(LUAJIT_PREFIX PREFIX=${RUNTIME_OUTPUT})

	if(APPLE)
        set(ENVIRONMENT_COMMAND export MACOSX_DEPLOYMENT_TARGET=$ENV{MACOSX_DEPLOYMENT_TARGET} &&)
    endif()

	set(OUTPUT_COMMAND 
		${ENVIRONMENT_COMMAND}
        ${MAKE_COMMAND} ${LUAJIT_PREFIX}
        && ${MAKE_COMMAND} install ${LUAJIT_PREFIX}
	)

endif() #platforms

file(GLOB LUAJIT_SRC CONFIGURE_DEPENDS ${CMAKE_SOURCE_DIR}/lib/luajit/src/*.c)

add_custom_command(OUTPUT ${OUTPUT_FILES}
    COMMAND ${OUTPUT_COMMAND}
    DEPENDS ${LUAJIT_SRC}
)

add_custom_target(luaJIT ALL DEPENDS ${OUTPUT_FILES})

add_custom_command(TARGET ECS PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/ECS/include ${RUNTIME_OUTPUT}/include
)

##################
##              ##
##    ECSlua    ##
##              ##
##################

add_library(ECSlua STATIC
    ECSlua/src/ECSlua.cpp
)

add_dependencies(ECSlua luaJIT)

if(WIN32 OR WIN64)
    set(LUAJIT_LIB ${RUNTIME_OUTPUT}/lib/lua51.lib)
else()
    set(LUAJIT_LIB ${RUNTIME_OUTPUT}/lib/libluajit-5.1.a)
endif()

target_include_directories(ECSlua PUBLIC ECSlua/include ${RUNTIME_OUTPUT}/include)
target_link_directories(ECSlua PUBLIC ${RUNTIME_OUTPUT}/lib ${CMAKE_BINARY_DIR})
target_link_libraries(ECSlua ECS ${LUAJIT_LIB})

set_target_properties(ECSlua PROPERTIES 
    CXX_STANDARD 17
)

add_custom_command(TARGET ECSlua PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/ECSlua/include ${RUNTIME_OUTPUT}/include
)

##################
##              ##
##    GLFW      ##
##              ##
##################

option(GLFW_BUILD_DOCS off)
option(GLFW_VULKAN_STATIC on)
add_subdirectory(lib/glfw ${CMAKE_BINARY_DIR}/glfw EXCLUDE_FROM_ALL)

add_custom_target(staticLibs)
add_dependencies(staticLibs glfw)

add_custom_command(TARGET staticLibs PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/lib/glfw/include/GLFW ${RUNTIME_OUTPUT}/include/GLFW/
)

##################
##              ##
##   gfx        ##
##              ##
##################

set(SOURCES gfx/src/gfx.cpp)

add_library(gfx STATIC ${SOURCES})
target_include_directories(gfx PUBLIC gfx/include ${Vulkan_INCLUDE_DIRS} ${CMAKE_SOURCE_DIR}/lib/glfw/include)

set_target_properties(gfx PROPERTIES 
    CXX_STANDARD 17
)

add_custom_command(TARGET gfx PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/gfx/include ${RUNTIME_OUTPUT}/include
)

##################
##              ##
##   Lutherie   ##
##              ##
##################

set(SOURCES src/lutherie.cpp)

add_library(lutherielib STATIC ${SOURCES})
add_dependencies(lutherielib staticLibs)

set_target_properties(lutherielib PROPERTIES 
	CXX_STANDARD 17
    OUTPUT_NAME lutherie
)

set(LINK_LIBS ECSlua glfw gfx)

target_link_directories(lutherielib PUBLIC ${CMAKE_BINARY_DIR}/lib/static)

target_include_directories(lutherielib PUBLIC ECSlua/include ${CMAKE_SOURCE_DIR}/include ${CMAKE_BINARY_DIR}/luajit/include)

target_link_libraries(lutherielib ${LINK_LIBS} ${Vulkan_LIBRARY})

add_custom_command(TARGET lutherielib PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/include ${RUNTIME_OUTPUT}/include
)

##################
##              ##
##   Launcher   ##
##              ##
##################

add_executable(lutherie src/main.cpp)

set_target_properties(lutherie PROPERTIES 
	CXX_STANDARD 17
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}
	ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
)

if(WIN32 OR WIN64)
	# set_target_properties(lutherie PROPERTIES LINK_FLAGS "/SUBSYSTEM:WINDOWS")
    add_definitions(-D_CRT_SECURE_NO_WARNINGS -DNODEFAULTLIB:library)
endif()

if(APPLE)
    add_custom_command(TARGET lutherie POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory $ENV{VULKAN_SDK}/etc ${RUNTIME_OUTPUT}/etc
        COMMAND ${CMAKE_COMMAND} -E copy_directory $ENV{VULKAN_SDK}/bin ${RUNTIME_OUTPUT}/bin
        COMMAND ${CMAKE_COMMAND} -E copy_directory $ENV{VULKAN_SDK}/lib ${RUNTIME_OUTPUT}/lib
        COMMAND ${CMAKE_COMMAND} -E copy_directory $ENV{VULKAN_SDK}/include/vulkan ${RUNTIME_OUTPUT}/include/vulkan
    )
    add_definitions(-DLUTHERIE_MAC -x objective-c++)
    set_target_properties(lutherie PROPERTIES 
        MACOSX_BUNDLE TRUE 
        MACOSX_BUNDLE_INFO_PLIST ${CMAKE_SOURCE_DIR}/Info.plist
    )
    target_link_options(lutherie PUBLIC -pagezero_size 10000 -image_base 100000000)
    target_link_libraries(lutherie ${COCOA_LIB} ${IOKIT_LIB} ${COREFOUNDATION_LIB} ${COREVIDEO_LIB})
endif()

target_link_directories(lutherie PUBLIC ${CMAKE_BINARY_DIR}/lib/static)
target_link_libraries(lutherie lutherielib)
target_include_directories(lutherie PUBLIC ${CMAKE_SOURCE_DIR}/include)

##################
##              ##
##  Lua files   ##
##              ##
##################

add_custom_command(TARGET lutherielib PRE_BUILD COMMAND ${CMAKE_COMMAND} -E make_directory ${RUNTIME_OUTPUT}/lib/lua)

set(LIB_LUA_DIR lib/lua)

function(add_lua_file LUA_NAME)
    add_custom_command(OUTPUT ${RUNTIME_OUTPUT}/${LIB_LUA_DIR}/${LUA_NAME}.raw 
        COMMAND ${RUNTIME_OUTPUT}/bin/luajit ARGS -b ${CMAKE_SOURCE_DIR}/scripts/${LUA_NAME}.lua ${RUNTIME_OUTPUT}/${LIB_LUA_DIR}/${LUA_NAME}.raw
        DEPENDS ${CMAKE_SOURCE_DIR}/scripts/${LUA_NAME}.lua)
    add_custom_target(${LUA_NAME}.raw ALL DEPENDS ${RUNTIME_OUTPUT}/${LIB_LUA_DIR}/${LUA_NAME}.raw)
    
    add_dependencies(${LUA_NAME}.raw lutherie)
endfunction()

add_lua_file(lutherie)

##################
##              ##
##   Modules    ##
##              ##
##################

add_custom_command(OUTPUT ${RUNTIME_OUTPUT}/modules/main.cpp
    COMMAND ${CMAKE_COMMAND} -E make_directory ${RUNTIME_OUTPUT}/modules
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/modules/main.cpp ${RUNTIME_OUTPUT}/modules/main.cpp
    DEPENDS ${CMAKE_SOURCE_DIR}/modules/main.cpp
)

add_custom_target(mainModule ALL DEPENDS ${RUNTIME_OUTPUT}/modules/main.cpp)

if(UNIX AND NOT APPLE)
    file(GLOB LINUX_BINS ${CMAKE_SOURCE_DIR}/bin/linux/appimagetool*.AppImage)
    add_custom_target(copyBin 
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/bin/linux/ ${RUNTIME_OUTPUT}/bin/linux/
        SOURCES ${LINUX_BINS}
    )
    add_dependencies(lutherie copyBin)
    
endif()

file(GLOB LUTHERIE_ICONS ${CMAKE_SOURCE_DIR}/resources/icons/*.?)
add_custom_target(copyIcons ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/resources/icons/ ${RUNTIME_OUTPUT}/resources/icons/
    SOURCES ${LUTHERIE_ICONS}
)

add_custom_command(TARGET ECS PRE_BUILD COMMAND ${CMAKE_COMMAND} -E make_directory ${RUNTIME_OUTPUT}/include)
