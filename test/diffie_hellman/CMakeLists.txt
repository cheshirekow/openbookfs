

find_package(Crypto++)
                             
                             
if( (Crypto++_FOUND)
    )
                                                                    
    include_directories( 
        ${Crypto++_INCLUDE_DIR}
        )
    
    
    set(LIBS ${LIBS} 
        ${Crypto++_LIBRARY}
        )
        
    add_executable( diffie_hellman_test
                    diffie_hellman_test.cpp
                             )
                            
    target_link_libraries( diffie_hellman_test ${LIBS})
    
else() 

    set(MISSING, "")
    
    if( NOT (Crypto++_FOUND) )
        set(MISSING "${MISSING} crypto++,")
    endif()
    
    message( WARNING "Can't build diffie_hellman_test, missing: ${MISSING}")

endif()

