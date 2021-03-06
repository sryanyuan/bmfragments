CMAKE_MINIMUM_REQUIRED(VERSION 2.8.2)

# Project name
PROJECT (bmserver)

# Default options
ADD_COMPILE_OPTIONS(-std=c++11 -fno-builtin-memcmp -msse -msse4.2 -Wall)

# Defines
#ADD_DEFINITIONS(-DASIO_STANDALONE)

# Link libraries
# Zlib
#FIND_LIBRARY(ZLIB_LIB libz.a ${PROJECT_SOURCE_DIR}/lib/)
#LINK_LIBRARIES(${ZLIB_LIB})

IF(NOT DEFINED CMAKE_BUILD_TYPE)
    message("CMAKE_BUILD_TYPE is not set, build <Release> version by default")
    SET(CMAKE_BUILD_TYPE Release)
ENDIF()

IF(CMAKE_BUILD_TYPE STREQUAL "Debug")
    ADD_DEFINITIONS(-D_DEBUG -fstack-protector -g -O0)
ELSE()
    ADD_DEFINITIONS(-DNDEBUG -O3)
ENDIF()

# Header files
INCLUDE_DIRECTORIES(
    ${PROJECT_SOURCE_DIR}/CMainServer
    ${PROJECT_SOURCE_DIR}/GameWorld
    ${PROJECT_SOURCE_DIR}/IOServer
    ${PROJECT_SOURCE_DIR}/Interface
    ${PROJECT_SOURCE_DIR}/common
)

# Add source files
AUX_SOURCE_DIRECTORY(CMainServer SRC_LIST)
AUX_SOURCE_DIRECTORY(GameWorld SRC_LIST)
AUX_SOURCE_DIRECTORY(IOServer SRC_LIST)
AUX_SOURCE_DIRECTORY(common SRC_LIST)
AUX_SOURCE_DIRECTORY(tolua SRC_LIST)

ADD_EXECUTABLE(bmserver ${SRC_LIST})
