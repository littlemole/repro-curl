############################################
cmake_minimum_required(VERSION 2.8.9)
project(reprocurl)
include(CTest)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(INSTALL_DEBUG "On")
set(INSTALL_RELEASE "On")

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

set( CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} ${LINKERFLAGS}" )

message(" COMPILER: ${CMAKE_CXX_COMPILER_ID}")
message(" FLAGS: ${FLAGS}")

############################################
# OS support
############################################


IF (WIN32)

	set(OSLIBS "Ws2_32" "Rpcrt4")

	find_library(CURL_LIBRARY NAMES libcurl)
	find_library(NG2HTTP_LIBRARY NAMES nghttp2)
	find_library(LIBEVENT_LIBRARY NAMES event)
    find_library(PRIOCPP_LIBRARY NAMES priocpp)
    find_library(PRIOCPPD_LIBRARY NAMES priocppd)

	set(BUILDFLAGS "-await ${FLAGS}")
	set(DEBUGFLAGS -DMOL_PROMISE_DEBUG)

	set(LIBS ${GTEST_LIBRARIES} ${OPENSSL_LIBRARIES} ${NG2HTTP_LIBRARY} ${CURL_LIBRARY} ${STDLIB}  )

	if( "${CMAKE_BUILD_TYPE}" STREQUAL "Release")
		set(INSTALL_DEBUG "Off")
		set(LIBS ${LIBS} ${CMAKE_PROJECT_NAME} ${PRIOCPP_LIBRARY} )
		set(DEBUGFLAGS " ")
	else()
		set(INSTALL_RELEASE "Off")
		set(LIBS ${LIBS} ${CMAKE_PROJECT_NAME}d ${PRIOCPPD_LIBRARY} )
		set(DEBUGFLAGS "-DMOL_PROMISE_DEBUG ")
	endif()

 ELSEIF(UNIX)

    set(OSLIBS "pthread")

    find_package(PkgConfig)
    pkg_check_modules(CURL REQUIRED libcurl)
    pkg_check_modules(NGHTTP2 REQUIRED libnghttp2)
    pkg_check_modules(PRIOCPP REQUIRED priocpp)
    pkg_check_modules(LIBEVENT REQUIRED libevent_pthreads)

	set(LIBEVENT_LIBRARY ${LIBEVENT_LIBRARIES})

    set(BUILDFLAGS " ${FLAGS}")
	set(DEBUGFLAGS -g -DMOL_PROMISE_DEBUG)

	set(LIBS ${CMAKE_PROJECT_NAME}d ${PRIOCPP_LIBRARIES} ${GTEST_LIBRARIES} ${OPENSSL_LIBRARIES} ${NGHTTP2_LIBRARIES} ${CURL_LIBRARIES} ${STDLIB} )
ENDIF ()

############################################
# macros
############################################

macro(AddCompilerFlags target flags)
    get_target_property(CF ${target} COMPILE_FLAGS)
    if(CF STREQUAL "CF-NOTFOUND")
        SET(CF "") # set to empty string
    else()
        SET(CF "${CF} ") # a space to cleanly separate from existing content
    endif()

    SET(CF "${CF} ${flags}" )
    set_target_properties(${target} PROPERTIES COMPILE_FLAGS ${CF} )
endmacro()

############################################
# defaults
############################################


set(BACKEND ${LIBEVENT_LIBRARY})

option(WITH_LIBEVENT "use libevent (default:on)" ON)
if(WITH_LIBEVENT)
    add_definitions(-DPROMISE_USE_LIBEVENT)
else()
    add_definitions(-DPROMISE_USE_BOOST_ASIO)
    set(BACKEND "boost_system")
endif()


############################################
# sources
############################################

include_directories(include)
include_directories(${OPENSSL_INCLUDE_DIR})

file(GLOB SOURCES "src/*.cpp")
file(GLOB HEADERS "include/reprocurl/*.h")

############################################
# targets
############################################

add_library(reprocurl  STATIC ${SOURCES})
add_library(reprocurld STATIC ${SOURCES})

add_executable(Tests t/test.cpp)

AddCompilerFlags(reprocurl ${BUILDFLAGS})

AddCompilerFlags(reprocurld ${BUILDFLAGS})
AddCompilerFlags(reprocurld ${DEBUGFLAGS})

AddCompilerFlags(Tests ${BUILDFLAGS})
AddCompilerFlags(Tests ${DEBUGFLAGS})


############################################
# libs
############################################

target_link_libraries(Tests ${LIBS} ${BACKEND} ${OSLIBS} )


############################################
# tests
############################################

add_test(NAME AllTests COMMAND Tests)

file(COPY pem DESTINATION .)
 
############################################
# install rules
############################################
 
if( "${INSTALL_RELEASE}" STREQUAL "On")
	install(TARGETS reprocurl DESTINATION lib)
endif()

if( "${INSTALL_DEBUG}" STREQUAL "On")
	install(TARGETS reprocurld DESTINATION lib)
endif()


install(FILES ${HEADERS} DESTINATION include/reprocurl)

install(FILES reprocurl.pc DESTINATION lib/pkgconfig)
