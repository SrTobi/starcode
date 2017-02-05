cmake_minimum_required( VERSION 3.0 )

project(StarCode VERSION 0.1)
set(Project_SHORTCUT "sc")
set(Project_PREFIX "cs")
set(Project_MEDIA_DIR "media")

add_subdirectory("extras/buildx")
#buildx_enable_debug(all)
buildx_set_default_prefixes()
buildx_set_default_output_dirs()
buildx_activate_cpp11()
#buildx_enable_pch()
################### find packages ###################
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/extras/cmake/Modules")

if(WIN32)
	SET(Boost_USE_STATIC_LIBS ON)
endif()
find_package(Boost COMPONENTS system thread regex filesystem program_options REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})
link_directories(${Boost_LIBRARY_DIRS})

find_package(utilx REQUIRED)

#include_directories("tests")

# tests must come first
add_subdirectory("src")
