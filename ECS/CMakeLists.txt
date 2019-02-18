cmake_minimum_required(VERSION 3.13.3)
project(ECS)

add_library(ECS STATIC src/ECS.cpp)

set_target_properties(ECS PROPERTIES 
	PUBLIC_HEADER include/ECS.hpp
	CXX_STANDARD 17
	)

target_include_directories(ECS PUBLIC include)

#install(TARGETS ECS ARCHIVE DESTINATION ../libs PUBLIC_HEADER DESTINATION ../include)
