find_package(Fuse REQUIRED)
find_package(Boost COMPONENTS filesystem)
                                                                    
include_directories( 
    ${FUSE_INCLUDE_DIR}
    ${Boost_INCLUDE_DIRS}
    )


set(LIBS ${LIBS} 
    ${FUSE_LIBRARY}
    ${Boost_LIBRARIES}
    )

add_executable( ${CMAKE_PROJECT_NAME} 
                        main.cpp
                        singleton.cpp 
                        OpenbookFS.cpp)
                        
set_target_properties( ${CMAKE_PROJECT_NAME} PROPERTIES COMPILE_FLAGS "-D_FILE_OFFSET_BITS=64" )

target_link_libraries( ${CMAKE_PROJECT_NAME} ${LIBS})

