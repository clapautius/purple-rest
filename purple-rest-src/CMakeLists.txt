cmake_minimum_required (VERSION 2.6)

project (Purple-REST)

find_package(PkgConfig REQUIRED)

add_library (purple-rest SHARED
  purple-rest.c rest-api.cpp purple-interaction.cpp)

# find glib stuff
pkg_check_modules(PKG_GLIB glib-2.0)
find_path(GLIB2_MAIN_INCLUDE_DIR glib.h
  PATH_SUFFIXES glib-2.0
  PATHS ${PKG_GLIB_INCLUDE_DIRS} )

# search the glibconfig.h include dir under the same root where the library is found
find_library(GLIB2_LIBRARIES
  NAMES glib-2.0
  PATHS ${PKG_GLIB_LIBRARY_DIRS} )

find_path(GLIB2_INTERNAL_INCLUDE_DIR glibconfig.h
  PATH_SUFFIXES glib-2.0/include
  PATHS ${PKG_GLIB_INCLUDE_DIRS} ${PKG_GLIB_LIBRARIES} ${CMAKE_SYSTEM_LIBRARY_PATH})

# find jsoncpp stuff
find_path (JSONCPP_INCLUDE_DIR NAMES "json/config.h" PATH_SUFFIXES jsoncpp)
message (STATUS "JSONCPP_INCLUDE_DIR = ${JSONCPP_INCLUDE_DIR}")

# find curl stuff
pkg_search_module(CURL REQUIRED libcurl)
target_link_libraries(purple-rest ${CURL_LIBRARIES})
target_include_directories(purple-rest PUBLIC ${CURL_INCLUDE_DIRS})
target_compile_options(purple-rest PUBLIC ${CURL_CFLAGS_OTHER})

# find libpurple stuff
pkg_search_module(PURPLE REQUIRED purple)
target_link_libraries(purple-rest ${PURPLE_LIBRARIES})
target_include_directories(purple-rest PUBLIC ${PURPLE_INCLUDE_DIRS})
target_compile_options(purple-rest PUBLIC ${PURPLE_CFLAGS_OTHER})

include_directories (purple-rest ${GLIB_INCLUDE_DIR})
include_directories (purple-rest ${GLIB_INTERNAL_INCLUDE_DIR})
include_directories (purple-rest ${JSONCPP_INCLUDE_DIR})
