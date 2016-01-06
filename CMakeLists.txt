# root CMakeLists.txt for test-time project
# 2012/12/30 - Make it a CMake build
# Oct 10  2009 Original cut

cmake_minimum_required (VERSION 2.8)

project (test_time)

# The version number.
set (tt_MAJOR 1)
set (tt_MINOR 0)
set (tt_AGE   0)

# to distinguish between debug and release lib in windows
set( CMAKE_DEBUG_POSTFIX "d" ) # little effect in unix

if(CMAKE_COMPILER_IS_GNUCXX)
    set(WARNING_FLAGS_CXX "-Wall")
    set(WARNING_FLAGS_C   "-Wall")
endif(CMAKE_COMPILER_IS_GNUCXX)

if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang") 
   set (WARNING_FLAGS_CXX "-Wall -Wno-overloaded-virtual")
   set (WARNING_FLAGS_C   "-Wall")
endif() 

if(WIN32)
    if(MSVC)
        # turn off various warnings
        # foreach(warning 4244 4251 4267 4275 4290 4786 4305 4996)
        #     SET(WARNING_FLAGS "${WARNING_FLAGS} /wd${warning}")
        # endforeach(warning)
        set(MSVC_FLAGS "-DNOMINMAX -D_USE_MATH_DEFINES -D_CRT_SECURE_NO_WARNINGS -D_SCL_SECURE_NO_WARNINGS -D__CRT_NONSTDC_NO_WARNINGS")
        #if (${MSVC_VERSION} EQUAL 1600)
        #    set( MSVC_LD_FLAGS "/FORCE:MULTIPLE" )
        #endif (${MSVC_VERSION} EQUAL 1600)
    endif(MSVC)
    set(NOMINMAX 1)
else()
    set(WARNING_FLAGS "${WARNING_FLAGS} -Wno-write-strings -Wno-unused-but-set-variable -Wno-unused-variable")
endif()

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${WARNING_FLAGS} ${MSVC_FLAGS} -D_REENTRANT")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${WARNING_FLAGS} ${MSVC_FLAGS} -D_REENTRANT ${BOOST_CXX_FLAGS}")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${MSVC_LD_FLAGS}")

include_directories ("${PROJECT_SOURCE_DIR}/src")
# include_directories ("${PROJECT_SOURCE_DIR}/src" "${PROJECT_SOURCE_DIR}/src/cf_lib" "${PROJECT_SOURCE_DIR}/src/tools")

# add a VERSION define
add_definitions( -DVERSION="${tt_MAJOR}.${tt_MINOR}.${tt_AGE}" )

set (tt_SRCS src/test-timer.cxx 
    src/test-utils.cxx 
    src/sprtf.cxx 
    src/MyTimer.cxx 
    src/test-time2.cxx )
set (tt_HDRS src/test-timer.hxx
    src/test-utils.hxx
    src/sprtf.hxx
    src/MyTimer.hxx
    src/win_Timer.hxx)
add_executable( test_time ${tt_SRCS} ${tt_HDRS} )
set_target_properties(test_time PROPERTIES DEBUG_POSTFIX "d" )
install( TARGETS test_time DESTINATION bin )

# eof
