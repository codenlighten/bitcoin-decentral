# cmake/modules/FindBerkeleyDB.cmake
# Outputs:
#   BerkeleyDB_FOUND
#   BerkeleyDB_INCLUDE_DIRS
#   BerkeleyDB_LIBRARIES
#   BerkeleyDB::BerkeleyDB (imported target)

include(CheckIncludeFileCXX)

# Try pkg-config first
find_package(PkgConfig QUIET)
if(PKG_CONFIG_FOUND)
  pkg_check_modules(PC_BDB QUIET db_cxx)
endif()

# Headers
find_path(BerkeleyDB_INCLUDE_DIRS
  NAMES db_cxx.h db.h
  HINTS
    ${PC_BDB_INCLUDE_DIRS}
    /usr/include
    /usr/local/include
)

# Libs (C++ first, then C)
find_library(BerkeleyDB_CXX_LIBRARY
  NAMES db_cxx db_cxx-4.8 db_cxx-5.3
  HINTS ${PC_BDB_LIBRARY_DIRS} /usr/lib /usr/local/lib
)

find_library(BerkeleyDB_C_LIBRARY
  NAMES db db-4.8 db-5.3
  HINTS ${PC_BDB_LIBRARY_DIRS} /usr/lib /usr/local/lib
)

set(BerkeleyDB_LIBRARIES "")
if(BerkeleyDB_CXX_LIBRARY)
  list(APPEND BerkeleyDB_LIBRARIES ${BerkeleyDB_CXX_LIBRARY})
endif()
if(BerkeleyDB_C_LIBRARY)
  list(APPEND BerkeleyDB_LIBRARIES ${BerkeleyDB_C_LIBRARY})
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(BerkeleyDB
  REQUIRED_VARS BerkeleyDB_INCLUDE_DIRS BerkeleyDB_LIBRARIES
)

# Create imported target
if(BerkeleyDB_FOUND AND NOT TARGET BerkeleyDB::BerkeleyDB)
  add_library(BerkeleyDB::BerkeleyDB INTERFACE IMPORTED)
  set_target_properties(BerkeleyDB::BerkeleyDB PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES "${BerkeleyDB_INCLUDE_DIRS}"
    INTERFACE_LINK_LIBRARIES "${BerkeleyDB_LIBRARIES}"
  )
endif()

mark_as_advanced(BerkeleyDB_INCLUDE_DIRS BerkeleyDB_CXX_LIBRARY BerkeleyDB_C_LIBRARY)
