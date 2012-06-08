find_package(Fuse REQUIRED)
                                                                    
include_directories( 
    ${FUSE_INCLUDE_DIR}
    )


set(LIBS ${LIBS} 
    ${FUSE_LIBRARY}
    )

add_executable( ${CMAKE_PROJECT_NAME} 
                        main.cpp
                        singleton.cpp 
                        OpenbookFS.cpp)
                        
set_target_properties( ${CMAKE_PROJECT_NAME} PROPERTIES COMPILE_FLAGS "-D_FILE_OFFSET_BITS=64" )

target_link_libraries( ${CMAKE_PROJECT_NAME} ${LIBS})

