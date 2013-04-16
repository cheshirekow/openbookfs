find_package(Fuse)
find_package(Boost COMPONENTS filesystem)
find_package(Threads)
find_package(CPPThreads)
find_package(Protobuf)
find_package(Crypto++)
find_package(RE2)
find_package(Tclap)
find_package(YamlCpp)
find_package(Soci COMPONENTS sqlite3) 
find_package(SigC++)
configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/config.yaml
                ${CMAKE_CURRENT_BINARY_DIR}/config.yaml )
                             
if( (FUSE_FOUND)
    AND (Boost_FOUND)
    AND (THREADS_FOUND)
    AND (CPPThreads_FOUND)
    AND (Protobuf_FOUND)
    AND (Crypto++_FOUND)
    AND (RE2_FOUND)
    AND (Tclap_FOUND) 
    AND (YAMLCPP_FOUND)
    AND (THREADS_FOUND)
    AND (SOCI_FOUND)
    AND (SigC++_FOUND)
     )
                                                                    
    include_directories( 
        ${FUSE_INCLUDE_DIRS}
        ${Boost_INCLUDE_DIRS}
        ${THREADS_INCLUDE_DIRS}
        ${CPPThreads_INCLUDE_DIR}
        ${Protobuf_INCLUDE_DIR}
        ${Crypto++_INCLUDE_DIR}
        ${RE2_INCLUDE_DIR}
        ${Tclap_INCLUDE_DIR}
        ${YAMLCPP_INCLUDE_DIR}
        ${SOCI_INCLUDE_DIRS}
        ${SigC++_INCLUDE_DIRS}
        ${CMAKE_CURRENT_SOURCE_DIR}
        )
    
    
    set(LIBS ${LIBS} 
        ${CMAKE_THREAD_LIBS_INIT}
        ${FUSE_LIBRARY}
        ${Boost_LIBRARIES}
        ${Protobuf_LIBRARY}
        ${CPPThreads_LIBRARY}
        ${THREADS_LIBRARIES}
        ${Crypto++_LIBRARY}
        ${RE2_LIBRARY}
        ${YAMLCPP_LIBRARY}
        ${SigC++_LIBRARIES}
        ${SOCI_LIBRARY}
        ${SOCI_sqlite3_PLUGIN}
        )
        
    add_executable( obfs_backend 
                    main.cpp
                    Backend.cpp
                    Connection.cpp
                    FuseContext.cpp
                    LongJob.cpp
                    MessageHandler.cpp
                    SocketListener.cpp
                    ../messages.cpp
                    ../FdSet.cpp
                    ../FileDescriptor.cpp
                    ../Marshall.cpp
                    ../NotifyPipe.cpp
                    ../SelectSpec.cpp
                    ../TimeVal.cpp
                    ${MESSAGES_PB_CC}
                    )
                            
    set_target_properties( obfs_backend 
        PROPERTIES COMPILE_FLAGS "-D_FILE_OFFSET_BITS=64 -std=gnu++0x" )
    
    target_link_libraries( obfs_backend ${LIBS})
    
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
    
    if( NOT (Crypto++_FOUND) )
        set(MISSING "${MISSING} crypto++,")
    endif()
    
    if( NOT (RE2_FOUND) )
        set(MISSING "${MISSING} re2,")
    endif()
    
    if( NOT (Tclap_FOUND) )
        set(MISSING "${MISSING} tclap,")
    endif()
    
    if( NOT (Soci_FOUND) )
        set(MISSING "${MISSING} soci,")
    endif()
    
    if( NOT (SigC++_FOUND) )
        set(MISSGIN "${MISSING} sigc++,")
    endif()
    
    message( WARNING "Can't build obfs_backend, missing: ${MISSING}")

endif()



