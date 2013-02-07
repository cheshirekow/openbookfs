find_package(Fuse)
find_package(Boost COMPONENTS filesystem)
find_package(Threads)
find_package(CPPThreads)
                             
                             
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
                            main.cpp
                            singleton.cpp 
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

