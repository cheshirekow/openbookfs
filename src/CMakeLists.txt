find_package( Protobuf )

if( (Protobuf_FOUND) )

    include_directories( 
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_BINARY_DIR} )

    protoc( messages.proto )
    set( MESSAGES_PB_CC ${CMAKE_CURRENT_BINARY_DIR}/messages.pb.cc )
    add_custom_target(proto_messages DEPENDS ${MESSAGES_PB_CC})
    
    add_subdirectory(client_fs)
    add_subdirectory(server)

endif()
