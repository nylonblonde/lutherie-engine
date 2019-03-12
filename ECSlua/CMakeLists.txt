cmake_minimum_required(VERSION 3.13.3)
project(ECSlua)

set(LUA_LIB_DIR /usr/local/lib CACHE PATH "")
set(LUA_INCLUDE_DIR /usr/local/include CACHE PATH "")

set(LINK_LIBS luajit ECS)

add_library(ECSlua SHARED src/ECSlua.cpp)

get_filename_component(DIR_ABOVE ../ ABSOLUTE)

set_target_properties(ECSlua PROPERTIES 
	PUBLIC_HEADER include/ECSlua.hpp
	CXX_STANDARD 17
	)

target_link_directories(ECSlua PUBLIC ${DIR_ABOVE}/ECS/build ${LUA_LIB_DIR})
target_include_directories(ECSlua PUBLIC include ${DIR_ABOVE}/ECS/include ${LUA_INCLUDE_DIR})

target_link_libraries(ECSlua ${LINK_LIBS})

#install(TARGETS ECS LIBRARY DESTINATION /usr/local/lib)
