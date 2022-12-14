PROJECT(opentok-alsa-out-via-subscriber-audio-data)

SET(CMAKE_VERBOSE_MAKEFILE ON)
# SET(ENV{LIBOPENTOK_PATH} /usr/lib/libopentok_linux_llvm_arm64)
CMAKE_MINIMUM_REQUIRED(VERSION 3.5)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -stdlib=libc++")

# Remove the commented out line below to enable OpenTok debug traces.
# SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DCONSOLE_LOGGING")

INCLUDE(FindPkgConfig)
PKG_SEARCH_MODULE(LIBSDL2 REQUIRED sdl2)
#PKG_SEARCH_MODULE(LIBOPENTOK REQUIRED libopentok)
if (DEFINED ENV{LIBOPENTOK_PATH})
    find_path(LIBOPENTOK_HEADER opentok.h PATHS $ENV{LIBOPENTOK_PATH}/include NO_DEFAULT_PATH)
    find_library(LIBOPENTOK_LIBRARIES libopentok NAMES libopentok.so PATHS $ENV{LIBOPENTOK_PATH}/lib NO_DEFAULT_PATH)
endif ()

if (NOT LIBOPENTOK_LIBRARIES AND NOT LIBOPENTOK_HEADER)
    PKG_SEARCH_MODULE(LIBOPENTOK REQUIRED libopentok)
else ()
    set(LIBOPENTOK_LIBRARY_DIRS $ENV{LIBOPENTOK_PATH}/lib)
    set(LIBOPENTOK_INCLUDE_DIRS $ENV{LIBOPENTOK_PATH}/include)
endif ()

FIND_PACKAGE(Threads)

find_package(ALSA REQUIRED)                                    
if (ALSA_FOUND)                                                
    include_directories(${ALSA_INCLUDE_DIRS})
endif(ALSA_FOUND) 

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/../common/src ${LIBSDL2_INCLUDE_DIRS} ${LIBOPENTOK_INCLUDE_DIRS})
LINK_DIRECTORIES(${LIBSDL2_LIBRARY_DIRS} ${LIBOPENTOK_LIBRARY_DIRS})

ADD_EXECUTABLE(${PROJECT_NAME} main.cpp ${CMAKE_CURRENT_SOURCE_DIR}/../common/src/otk_thread.c)
TARGET_LINK_LIBRARIES(${PROJECT_NAME} pthread ${ALSA_LIBRARIES} ${LIBSDL2_LIBRARIES} ${LIBOPENTOK_LIBRARIES})
