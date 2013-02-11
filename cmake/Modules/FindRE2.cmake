# - Try to find RE2
# Once done, this will define
#
#  RE2_FOUND - system has re2
#  RE2_INCLUDE_DIRS - the re2 include directories
#  RE2_LIBRARIES - link these to use re2

include(LibFindMacros)

# Main include dir
find_path(RE2_INCLUDE_DIR
  NAMES re2/re2.h
)

libfind_library(RE2 re2)

# Set the include dir variables and the libraries and let libfind_process do the rest.
# NOTE: Singular variables for this library, plural for libraries this this lib depends on.
set(RE2_PROCESS_INCLUDES 
        RE2_INCLUDE_DIR )


set(RE2_PROCESS_LIBS 
        RE2_LIBRARY)

libfind_process(RE2)

