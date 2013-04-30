# - Try to find CPPThreads
# Once done, this will define
#
#  CPPThreads_FOUND        - system has CPPThreads
#  CPPThreads_INCLUDE_DIRS - the CPPThreads include directories
#  CPPThreads_LIBRARIES    - link these to use CPPThreads

include(LibFindMacros)

libfind_package( CPPThreads Threads )

# include dir
find_path(CPPThreads_INCLUDE_DIR
  NAMES cpp-pthreads.h
)

#library
libfind_library(CPPThreads cpp-pthreads)
libfind_library(CPPThreads_STATIC cpp-pthreads_static)

# Set the include dir variables and the libraries and let libfind_process do the rest.
# NOTE: Singular variables for this library, plural for libraries this this lib depends on.
set(CPPThreads_PROCESS_INCLUDES CPPThreads_INCLUDE_DIR)
set(CPPThreads_PROCESS_LIBS CPPThreads_LIBRARY)
libfind_process(CPPThreads)
set(CPPThreads_LIBRARIES ${CPPThreads_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT} )

