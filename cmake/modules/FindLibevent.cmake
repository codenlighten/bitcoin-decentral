# cmake/modules/FindLibevent.cmake
# Exposes:
#   Libevent_FOUND
#   Libevent_INCLUDE_DIRS
#   Libevent_LIBRARIES

include(FindPackageHandleStandardArgs)
find_package(PkgConfig QUIET)
if(PKG_CONFIG_FOUND)
  pkg_check_modules(PC_LIBEVENT QUIET libevent)
endif()

# Headers
find_path(Libevent_INCLUDE_DIRS
  NAMES event2/event.h
  HINTS
    ${PC_LIBEVENT_INCLUDE_DIRS}
    /usr/include
    /usr/local/include
)

# Core lib (event_core preferred; plain 'event' acceptable)
find_library(Libevent_CORE_LIBRARY
  NAMES event_core event
  HINTS ${PC_LIBEVENT_LIBRARY_DIRS} /usr/lib /usr/local/lib
)

# Pthreads helper (optional but commonly used)
find_library(Libevent_PTHREADS_LIBRARY
  NAMES event_pthreads
  HINTS ${PC_LIBEVENT_LIBRARY_DIRS} /usr/lib /usr/local/lib
)

set(Libevent_LIBRARIES "${Libevent_CORE_LIBRARY}")
if(Libevent_PTHREADS_LIBRARY)
  list(APPEND Libevent_LIBRARIES "${Libevent_PTHREADS_LIBRARY}")
endif()

find_package_handle_standard_args(Libevent
  REQUIRED_VARS Libevent_INCLUDE_DIRS Libevent_LIBRARIES
)

mark_as_advanced(Libevent_INCLUDE_DIRS Libevent_CORE_LIBRARY Libevent_PTHREADS_LIBRARY)
