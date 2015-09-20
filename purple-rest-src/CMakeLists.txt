cmake_minimum_required (VERSION 2.6)

project (PURPLE-REST)

find_package(PkgConfig REQUIRED)

SET(CMAKE_CXX_FLAGS "-std=c++11 -Wall ")

set (PURPLE_REST_SRC
  rest-api.cpp
  purple-interaction.cpp
  history.cpp
  immessage.cpp)

add_library (purple-rest SHARED ${PURPLE_REST_SRC} purple-rest.c)

add_executable (purple-rest-test ${PURPLE_REST_SRC} purple-rest-test.cpp)

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

include_directories (purple-rest ${GLIB_INCLUDE_DIR})
include_directories (purple-rest ${GLIB_INTERNAL_INCLUDE_DIR})
include_directories (purple-rest-test ${GLIB_INCLUDE_DIR})
include_directories (purple-rest-test ${GLIB_INTERNAL_INCLUDE_DIR})

# find jsoncpp stuff
pkg_search_module(JSONCPP REQUIRED jsoncpp)
target_link_libraries(purple-rest ${JSONCPP_LIBRARIES})
target_include_directories(purple-rest PUBLIC ${JSONCPP_INCLUDE_DIRS})
target_compile_options(purple-rest PUBLIC ${JSONCPP_CFLAGS_OTHER})
target_link_libraries(purple-rest-test ${JSONCPP_LIBRARIES})
target_include_directories(purple-rest-test PUBLIC ${JSONCPP_INCLUDE_DIRS})
target_compile_options(purple-rest-test PUBLIC ${JSONCPP_CFLAGS_OTHER})

# find libmicrohttpd stuff
pkg_search_module(MICROHTTPD REQUIRED libmicrohttpd)
target_link_libraries(purple-rest ${MICROHTTPD_LIBRARIES})
target_include_directories(purple-rest PUBLIC ${MICROHTTPD_INCLUDE_DIRS})
target_compile_options(purple-rest PUBLIC ${MICROHTTPD_CFLAGS_OTHER})
target_link_libraries(purple-rest-test ${MICROHTTPD_LIBRARIES})
target_include_directories(purple-rest-test PUBLIC ${MICROHTTPD_INCLUDE_DIRS})
target_compile_options(purple-rest-test PUBLIC ${MICROHTTPD_CFLAGS_OTHER})

# find libpurple stuff
pkg_search_module(PURPLE REQUIRED purple)
target_link_libraries(purple-rest ${PURPLE_LIBRARIES})
target_include_directories(purple-rest PUBLIC ${PURPLE_INCLUDE_DIRS})
target_compile_options(purple-rest PUBLIC ${PURPLE_CFLAGS_OTHER})
target_link_libraries(purple-rest-test ${PURPLE_LIBRARIES})
target_include_directories(purple-rest-test PUBLIC ${PURPLE_INCLUDE_DIRS})
target_compile_options(purple-rest-test PUBLIC ${PURPLE_CFLAGS_OTHER})
