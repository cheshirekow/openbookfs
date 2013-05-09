cmake_minimum_required(VERSION 2.8)

# defines the project name
project (openbookfs)
set( ${CMAKE_PROJECT_NAME}_VERSION_MAJOR 0 )
set( ${CMAKE_PROJECT_NAME}_VERSION_MINOR 1 )
set( ${CMAKE_PROJECT_NAME}_VERSION_BUGFIX 0 )

# adds the project-specific cmake module directory cmake/Modules to the cmake
# search path
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

# finds pkg-config
find_package(PkgConfig)

# add the src/ subdirectory to the list of directories cmake processes
include_directories(include)
add_subdirectory(src)
add_subdirectory(include)
add_subdirectory(test)

# configure the doxygen configuration
configure_file( 
    "${PROJECT_SOURCE_DIR}/doxy.config.in"
    "${PROJECT_BINARY_DIR}/doxy.config" 
    )
    
configure_file( 
    "${PROJECT_SOURCE_DIR}/doxystyle.css"
    "${PROJECT_BINARY_DIR}/doxystyle.css" 
    )

# use Jan Woetzel's doxygen doc target
include("TargetDoc" OPTIONAL)
