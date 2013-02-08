find_package(Fuse)
find_package(Boost COMPONENTS filesystem)
find_package(Threads)
find_package(CPPThreads)
find_package(Protobuf)
                             
                             
if( (FUSE_FOUND)
    AND (Boost_FOUND)
    AND (THREADS_FOUND)
    AND (CPPThreads_FOUND)
    AND (Protobuf_FOUND) )
                                                                    
    include_directories( 
        ${FUSE_INCLUDE_DIRS}
        ${Boost_INCLUDE_DIRS}
        ${THREADS_INCLUDE_DIRS}
        ${CPPThreads_INCLUDE_DIR}
        ${Protobuf_INCLUDE_DIR}
        )
    
    
    set(LIBS ${LIBS} 
        ${FUSE_LIBRARY}
        ${Boost_LIBRARIES}
        ${Protobuf_LIBRARY}
        )
        
     protoc( server.proto )
    
    add_executable( openbookfs_d
                    main.cpp
                    ${CMAKE_CURRENT_BINARY_DIR}/server.pb.h
                    ${CMAKE_CURRENT_BINARY_DIR}/server.pb.cc
                             )
                            
    target_link_libraries( openbookfs_d ${LIBS})
    
else() 

    set(MISSING, "")
    
    if( NOT (FUSE_FOUND) )
        set(MISSING "${MISSING} libfuse,")
    endif()
    
    if( NOT (Boost_FOUND) )
        set(MISSING "${MISSING} boost-filesystem,")
    endif()
    
    if( NOT (THREADS_FOUND) )
        set(MISSING "${MISSING} pthreads,")
    endif()
    
    if( NOT (CPPThreads_FOUND) )
        set(MISSING "${MISSING} cpp-pthreads,")
    endif()
    
    if( NOT (Protobuf_FOUND) )
        set(MISSING "${MISSING} protobuf,")
    endif()
    
    message( WARNING "Can't build openbookfs server, missing: ${MISSING}")

endif()

