# - Try to find Google Protocol Buffers
# Once done, this will define
#
#  Protobuf_FOUND        - system has protobuf
#  Protobuf_INCLUDE_DIR  - protobuf include path
#  Protobuf_LIBRARY      - libprotobuf path
#  Protobuf_LITE_LIBRARY - libprotobuf-lite path
#  Protobuf_COMPILER     - ptortoc path

include(LibFindMacros)

# Use pkg-config to get hints about paths
libfind_pkg_check_modules(Protobuf_PKGCONF protobuf)

# Find include directory
find_path(Protobuf_INCLUDE_DIR
  NAMES protobuf/message.h
  PATH_SUFFIXES google
  HINTS ${Protobuf_PKGCONF_INCLUDE_DIRS}
)

# Find librarys
find_library(Protobuf_LIBRARY
  NAMES protobuf
  HINTS ${Protobuf_PKGCONFIG_LIBRARY_DIRS}
)

find_library(Protobuf_LITE_LIBRARY
  NAMES protobuf-lite
  HINTS ${Protobuf_PKGCONFIG_LIBRARY_DIRS}
)

# Find compiler
find_program(Protobuf_COMPILER
  NAMES protoc
)

# Set the include dir variables and the libraries and let libfind_process do the rest.
# NOTE: Singular variables for this library, plural for libraries this this lib depends on.
set(Protobuf_PROCESS_INCLUDES Protobuf_INCLUDE_DIR)
set(Protobuf_PROCESS_LIBRARIES Protobuf_LIBRARY Protobuf_LITE_LIBRARY)
libfind_process(Protobuf)

if( NOT (Protobuf_COMPILER) )
   set( Protobuf_FOUND "-NOTFOUND" )
endif()


if( NOT (Protobuf_FOUND) )
   message( WARNING "Failed to find google protocol buffers:\n"
      "_INCLUDE_DIR  : ${Protobuf_INCLUDE_DIR}\n"
      "_LIBRARY      : ${Protobuf_LIBRARY}\n"
      "_LITE_LIBRARY : ${Protobuf_LITE_LIBRARY}\n"
      "_COMPILER     : ${Protobuf_COMPILER}\n" )
else()
  message( STATUS "Found google protocol buffers:\n"
      "_INCLUDE_DIR  : ${Protobuf_INCLUDE_DIR}\n"
      "_LIBRARY      : ${Protobuf_LIBRARY}\n"
      "_LITE_LIBRARY : ${Protobuf_LITE_LIBRARY}\n"
      "_COMPILER     : ${Protobuf_COMPILER}\n" )
endif()


function( protoc PROTO_FILES )
    set( OUTPUT_FILES "" ) 
    set( INPUT_FILES "" )
    foreach( PROTO_FILE ${PROTO_FILES} )
        string( REGEX REPLACE ".proto" ".pb.h"  HEADER_FILE ${PROTO_FILE} )
        string( REGEX REPLACE ".proto" ".pb.cc" SOURCE_FILE ${PROTO_FILE} )
        list( APPEND OUTPUT_FILES 
                ${CMAKE_CURRENT_BINARY_DIR}/${HEADER_FILE}
                ${CMAKE_CURRENT_BINARY_DIR}/${SOURCE_FILE} )

        list( APPEND INPUT_FILES ${CMAKE_CURRENT_SOURCE_DIR}/${PROTO_FILE} )
        
        message( STATUS "protoc:\n"
          "INPUT_FILES   : ${INPUT_FILES}\n"
          "OUTPUT_FILES  : ${OUTPUT_FILES}\n" )

        set_source_files_properties(
            ${CMAKE_CURRENT_BINARY_DIR}/${HEADER_FILE} GENERATED )        
        set_source_files_properties(
            ${CMAKE_CURRENT_BINARY_DIR}/${SOURCE_FILE} GENERATED )
    endforeach()
    
    message( STATUS "custom command:\n"
          "OUTPUT  : ${OUTPUT_FILES}\n"
          "command : ${Protobuf_COMPILER} -I=${CMAKE_CURRENT_SOURCE_DIR} --cpp_out=${CMAKE_CURRENT_BINARY_DIR} ${INPUT_FILES}\n" )
    add_custom_command( OUTPUT ${OUTPUT_FILES}
      COMMAND ${Protobuf_COMPILER} -I=${CMAKE_CURRENT_SOURCE_DIR} --cpp_out=${CMAKE_CURRENT_BINARY_DIR} ${INPUT_FILES}
      DEPENDS ${PROTO_FILES}
    )
endfunction()
