find_package(Boost COMPONENTS filesystem)
find_package(Threads)
find_package(CPPThreads)
find_package(Protobuf)
find_package(Crypto++)
find_package(Qt4 COMPONENTS QtCore QtGui QtNetwork QtSVG )
                             
if( (Boost_FOUND)
    AND (THREADS_FOUND)
    AND (CPPThreads_FOUND)
    AND (Protobuf_FOUND)
    AND (Crypto++_FOUND)
    AND (QT_FOUND)
     )
     
    SET(QT_USE_QTNETWORK TRUE)
    SET(QT_USE_QTSVG TRUE)
                          
    include_directories( 
        ${Boost_INCLUDE_DIRS}
        ${THREADS_INCLUDE_DIRS}
        ${CPPThreads_INCLUDE_DIR}
        ${Protobuf_INCLUDE_DIR}
        ${Crypto++_INCLUDE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_BINARY_DIR}
        )
    
    
    set(LIBS ${LIBS} 
        ${CMAKE_THREAD_LIBS_INIT}
        ${Boost_LIBRARIES}
        ${Protobuf_LIBRARY}
        ${CPPThreads_LIBRARY}
        ${THREADS_LIBRARIES}
        ${Crypto++_LIBRARY}
        ${QT_QTCORE_LIBRARY}
        ${QT_QTGUI_LIBRARY}
        ${QT_QTNETWORK_LIBRARY}
        ${QT_QTSVG_LIBRARY}
        )
        
    set( gui_SOURCES
            main.cpp
            mainwindow.cpp 
            localclient.cpp 
            info.cpp 
            ${MESSAGES_PB_CC}
            connection.cpp 
            Options.cpp 
            commands/StartSync.cpp 
            commands/SetRemoteSocket.cpp 
            commands/SetLocalSocket.cpp 
            commands/SetDisplayName.cpp 
            commands/SetClientSocket.cpp 
            commands/SaveConfig.cpp 
            commands/LoadConfig.cpp 
            commands/ListMounts.cpp 
            commands/ListKnownPeers.cpp 
            commands/Connect.cpp 
            commands/SetDataDir.cpp 
            ../TimeVal.cpp 
            ../TimeSpec.cpp 
            ../SelectSpec.cpp 
            ../NotifyPipe.cpp 
            ../messages.cpp 
            ../MessageBuffer.cpp 
            ../Marshall.cpp 
            ../FileDescriptor.cpp 
            ../FdSet.cpp 
            ../base64.cpp 
            commands/Release.cpp 
            commands/Checkout.cpp
            )

    set( gui_HEADERS  
            mainwindow.h 
            localclient.h 
            info.h 
            )

    set( gui_FORMS  
            mainwindow.ui 
            localclient.ui 
            )
            
    set( gui_RESOURCES
            resources.qrc 
            )
    
    QT4_WRAP_UI(  gui_FORMS_HEADERS ${gui_FORMS} )
    QT4_WRAP_CPP( gui_HEADERS_MOC ${gui_HEADERS} )        
    QT4_ADD_RESOURCES( gui_RESOURCES_RCC ${gui_RESOURCES} )
    include(${QT_USE_FILE})
    add_definitions(${QT_DEFINITIONS})
        
    add_executable( obfs_gui
                    ${gui_SOURCES}
                    ${gui_HEADERS_MOC}
                    ${gui_FORMS_HEADERS}
                    ${gui_RESOURCES_RCC}
                    )
                            
    set_target_properties( obfs_gui 
        PROPERTIES COMPILE_FLAGS "-D_FILE_OFFSET_BITS=64 -std=gnu++0x" )
    
    target_link_libraries( obfs_gui ${LIBS})
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
    
    if( NOT (QT_FOUND) )
        set(MISSING "${MISSING} qt4,")
    endif()
    
    message( WARNING "Can't build obfs_gui, missing: ${MISSING}")

endif()



