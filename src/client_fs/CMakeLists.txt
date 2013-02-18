find_package(Fuse)
find_package(Boost COMPONENTS filesystem)
find_package(Threads)
find_package(CPPThreads)
find_package(Protobuf)
find_package(Crypto++)
find_package(RE2)
find_package(Tclap)
find_package(YamlCpp)

configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/openbookfs_c.yaml
                ${CMAKE_CURRENT_BINARY_DIR}/openbookfs_c.yaml )
                             
if( (FUSE_FOUND)
    AND (Boost_FOUND)
    AND (THREADS_FOUND)
    AND (CPPThreads_FOUND) )
                                                                    
    include_directories( 
        ${FUSE_INCLUDE_DIRS}
        ${Boost_INCLUDE_DIRS}
        ${THREADS_INCLUDE_DIRS}
        ${CPPThreads_INCLUDE_DIR}
        
        )
    
    
    set(LIBS ${LIBS} 
        ${FUSE_LIBRARY}
        ${Boost_LIBRARIES}
        )
    
    add_executable( ${CMAKE_PROJECT_NAME} 
                            ../base64.cpp
                            main.cpp
                            fuse_operations.cpp
                            ExtendedAttributes.cpp
                            OpenbookFS.cpp)
                            
    set_target_properties( ${CMAKE_PROJECT_NAME} 
        PROPERTIES COMPILE_FLAGS "-D_FILE_OFFSET_BITS=64" )
    
    target_link_libraries( ${CMAKE_PROJECT_NAME} ${LIBS})
    
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
    
    message( WARNING "Can't build openbook client_fs, missing: ${MISSING}")

endif()



                             
if( (Boost_FOUND)
    AND (THREADS_FOUND)
    AND (CPPThreads_FOUND)
    AND (Protobuf_FOUND)
    AND (Crypto++_FOUND)
    AND (RE2_FOUND)
    AND (Tclap_FOUND) 
    AND (YAMLCPP_FOUND)
    )
                                                                    
    include_directories( 
        ${Boost_INCLUDE_DIRS}
        ${THREADS_INCLUDE_DIRS}
        ${CPPThreads_INCLUDE_DIR}
        ${Protobuf_INCLUDE_DIR}
        ${Crypto++_INCLUDE_DIR}
        ${RE2_INCLUDE_DIR}
        ${Tclap_INCLUDE_DIR}
        ${YAMLCPP_INCLUDE_DIR}
        )
    
    
    set(LIBS ${LIBS} 
        ${Boost_LIBRARIES}
        ${Protobuf_LIBRARY}
        ${CPPThreads_LIBRARY}
        ${THREADS_LIBRARIES}
        ${Crypto++_LIBRARY}
        ${RE2_LIBRARY}
        ${YAMLCPP_LIBRARY}
        )
        
    set_source_files_properties( ${MESSAGES_PB_CC} GENERATED )      
    
    add_executable( client_protocol_test
                    ../base64.cpp
                    ../jobs.cpp
                    ../messages.cpp
                    ../MessageBuffer.cpp
                    ../FdSet.cpp
                    ../Job.cpp
                    ../JobHandler.cpp
                    ../SelectSpec.cpp
                    ../NotifyPipe.cpp
                    ../TimeVal.cpp   
                    Client.cpp       
                    ServerHandler.cpp          
                    protocol_test.cpp
                    ${MESSAGES_PB_CC}
                             )
    add_dependencies(client_protocol_test proto_messages)
                            
    target_link_libraries( client_protocol_test ${LIBS})
    
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
    
    if( NOT (Crypto++_FOUND) )
        set(MISSING "${MISSING} crypto++,")
    endif()
    
    if( NOT (RE2_FOUND) )
        set(MISSING "${MISSING} re2,")
    endif()
    
    if( NOT (Tclap_FOUND) )
        set(MISSING "${MISSING} tclap,")
    endif()
    
    
    message( WARNING "Can't build openbookfs client protocol test, "
                     "missing: ${MISSING}")

endif()
