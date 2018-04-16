############################################
# preamble
############################################
cmake_minimum_required(VERSION 2.8.9)
cmake_policy(SET CMP0054 NEW)
project(reprocurl)
include(CTest)

############################################
# c++ std
############################################

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

############################################
# debug mode produces libs with d suffix
############################################

if(NOT CMAKE_DEBUG_POSTFIX)
  set(CMAKE_DEBUG_POSTFIX d)
endif()

############################################
# basic dependecies
############################################

find_package(GTest REQUIRED)
find_package(OpenSSL REQUIRED)


############################################
# clang support
############################################

set(LINKERFLAGS "")
set(STDLIB "")
set(FLAGS "")


if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    set(STDLIB "c++abi")
    set(LINKERFLAGS "-stdlib=libc++ -fcoroutines-ts  -std=c++14")
    set(FLAGS "-stdlib=libc++ -fcoroutines-ts -D_RESUMABLE_FUNCTIONS_SUPPORTED")
endif()



############################################
# OS support (Linux/Windows)
############################################

IF (WIN32)

	set(OSLIBS "Ws2_32" "Rpcrt4")

	find_library(LIBEVENT_LIBRARY NAMES event)
	find_library(CURL_LIBRARY NAMES libcurl)
	find_library(NG2HTTP_LIBRARY NAMES nghttp2)
	find_library(LIBEVENT_LIBRARY NAMES event)
    find_library(PRIOCPP_LIBRARY NAMES priocpp)
    find_library(PRIOCPPD_LIBRARY NAMES priocppd)


	set(BUILDFLAGS "-await ${FLAGS}")

	set(LIBS ${GTEST_LIBRARIES} ${OPENSSL_LIBRARIES} ${NG2HTTP_LIBRARY} ${CURL_LIBRARY} debug ${PRIOCPPD_LIBRARY} optimized ${PRIOCPP_LIBRARY} ${STDLIB}  )

	set(LIBS ${LIBS} debug ${CMAKE_PROJECT_NAME} optimized ${CMAKE_PROJECT_NAME} )

 ELSEIF (UNIX)

    set(OSLIBS "pthread")

    find_package(PkgConfig)
    pkg_check_modules(CURL REQUIRED libcurl)
    pkg_check_modules(NGHTTP2 REQUIRED libnghttp2)
    pkg_check_modules(PRIOCPP REQUIRED priocpp)
    pkg_check_modules(LIBEVENT REQUIRED libevent_pthreads)

    set(LIBEVENT_LIBRARY ${LIBEVENT_LIBRARIES})

    set(BUILDFLAGS " ${FLAGS}")

    set(LIBS debug priocpp optimized priocpp ${GTEST_LIBRARIES} ${OPENSSL_LIBRARIES} ${NG2HTTP_LIBRARY} ${CURL_LIBRARIES} ${STDLIB}  )

    set(LIBS debug ${CMAKE_PROJECT_NAME} optimized ${CMAKE_PROJECT_NAME} ${LIBS} )

ENDIF ()


############################################
# set linker and compiler flags
############################################

set(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} ${LINKERFLAGS}" )
set(CMAKE_CXX_FLAGS_DEBUG  	"${CMAKE_CXX_FLAGS_DEBUG} -DMOL_PROMISE_DEBUG")
set(CMAKE_CXX_FLAGS  		"${CMAKE_CXX_FLAGS} ${BUILDFLAGS}")


############################################
# backend: libevent or boost_asio
############################################


option(WITH_LIBEVENT "use libevent (default:on)" ON)
if(WITH_LIBEVENT)
    add_definitions(-DPROMISE_USE_LIBEVENT)    
    set(BACKEND ${LIBEVENT_LIBRARY})
else()
    add_definitions(-DPROMISE_USE_BOOST_ASIO)
    set(BACKEND "boost_system")
endif()

############################################
# include directories
############################################

include_directories(include)
include_directories(${OPENSSL_INCLUDE_DIR})
include_directories(${PRIOCPP_INCLUDE_DIRS})


############################################
# sources
############################################

file(GLOB SOURCES "src/*.cpp")
file(GLOB HEADERS "include/${CMAKE_PROJECT_NAME}/*.h")

############################################
# lib target
############################################

add_library(${CMAKE_PROJECT_NAME}  STATIC ${SOURCES})

############################################
# tests
############################################

add_executable(Tests t/test.cpp)

############################################
# libs for tests
############################################
message("${LIBS}")
message("${BACKEND}")
message("${OSLIBS}")

target_link_libraries(Tests ${LIBS} ${BACKEND} ${OSLIBS} )

############################################
# run tests
############################################


add_test(NAME AllTests COMMAND Tests)

file(COPY pem DESTINATION .)
 
############################################
# install rules
############################################

install(TARGETS ${CMAKE_PROJECT_NAME} DESTINATION lib)

install(FILES ${HEADERS} DESTINATION include/${CMAKE_PROJECT_NAME})

install(FILES ${CMAKE_PROJECT_NAME}.pc DESTINATION lib/pkgconfig)



