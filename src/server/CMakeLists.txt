find_package(Boost COMPONENTS filesystem)
find_package(Threads)
find_package(CPPThreads)
find_package(Protobuf)
find_package(Tclap)
find_package(Crypto++)
                             
                             
if( (Boost_FOUND)
    AND (THREADS_FOUND)
    AND (CPPThreads_FOUND)
    AND (Protobuf_FOUND)
    AND (Crypto++_FOUND)
    AND (Tclap_FOUND) 
    )
                                                                    
    include_directories( 
        ${Boost_INCLUDE_DIRS}
        ${THREADS_INCLUDE_DIRS}
        ${CPPThreads_INCLUDE_DIR}
        ${Protobuf_INCLUDE_DIR}
        ${Tclap_INCLUDE_DIR}
        ${Crypto++_INCLUDE_DIR}
        )
    
    
    set(LIBS ${LIBS} 
        ${Boost_LIBRARIES}
        ${Protobuf_LIBRARY}
        ${CPPThreads_LIBRARY}
        ${THREADS_LIBRARIES}
        ${Crypto++_LIBRARY}
        )
        
    set_source_files_properties( ${MESSAGES_PB_CC} GENERATED )      
        
    add_executable( openbookfs_d
                    main.cpp
                    Protocol.cpp
                    RequestHandler.cpp
                    Server.cpp
                    ../messages.cpp
                    ../MessageBuffer.cpp
                    ${MESSAGES_PB_CC}
                             )
    add_dependencies(openbookfs_d proto_messages)
                            
    target_link_libraries( openbookfs_d ${LIBS})
    
else() 

    set(MISSING, "")
    
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
    
    if( NOT (Tclap_FOUND) )
        set(MISSING "${MISSING} tclap,")
    endif()
    
    if( NOT (Crypto++_FOUND) )
        set(MISSING "${MISSING} crypto++,")
    endif()
    
    message( WARNING "Can't build openbookfs server, missing: ${MISSING}")

endif()

