add_subdirectory(diffie_hellman)

file( MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/backend/a/data )
file( MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/backend/a/mountPoint )
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/backend/a/obfs.yaml
    ${CMAKE_CURRENT_BINARY_DIR}/backend/a/obfs.yaml )
file( COPY        ${CMAKE_CURRENT_SOURCE_DIR}/backend/a/start.sh
      DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/backend/a )


file( MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/backend/b/data )
file( MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/backend/b/mountPoint )
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/backend/b/obfs.yaml
    ${CMAKE_CURRENT_BINARY_DIR}/backend/b/obfs.yaml )
file( COPY        ${CMAKE_CURRENT_SOURCE_DIR}/backend/b/start.sh
      DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/backend/b )