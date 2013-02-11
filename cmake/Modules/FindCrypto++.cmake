# - Try to find Crypto++
# Once done, this will define
#
#  Crypto++_FOUND        - system has Crypto++
#  Crypto++_INCLUDE_DIR  - the Crypto++ include directory

include(LibFindMacros)

# Use pkg-config to get hints about paths
libfind_pkg_check_modules(Crypto++_PKGCONF crypto++)

# Main include dir
find_path(Crypto++_INCLUDE_DIR
  NAMES crypto++/cryptlib.h
  HINTS ${Crypto++_PKGCONF_INCLUDE_DIRS}
)

# Library
find_library(Crypto++_LIBRARY
  NAMES crypto++
  HINTS ${Crypto++_PKGCONF_LIBRARY_DIRS}
)



# Set the include dir variables and the libraries and let libfind_process do the rest.
# NOTE: Singular variables for this library, plural for libraries this this lib depends on.
set(Crypto++_PROCESS_INCLUDES Crypto++_INCLUDE_DIR)
set(Crypto++_PROCESS_LIBS Crypto++_LIBRARY)
libfind_process(Crypto++)

