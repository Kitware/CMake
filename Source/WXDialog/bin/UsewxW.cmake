
MESSAGE("${CMAKE_CURRENT_FILE} is deprecated, please use FindwxWidgets.cmake instead.")


#
# UsewxW.cmake
# v1.0 2005-05-27
# Jerry Fath 2005
# Based on work by Jorgen Bodde
#

#
#  wxWidgets CMake helper
#

# These project specific variables should be set before including this file
#PROJECT_NAME
#PROJECT_SOURCE_FILES
#PROJECT_SOURCE_DIR
#PROJECT_BINARY_DIR
# These variables can be set to add addition directories, libraries, and definitions
#PROJECT_DEFINITIONS
#PROJECT_INCLUDE_PATH
#PROJECT_LINK_DIRECTORIES
#PROJECT_LIBRARIES

#These WX specific variable should be set by FindwxW.cmake before including this file
# WXWIDGETS_LINK_DIRECTORIES = link directories, useful for rpath on Unix
# WXWIDGETS_LIBRARIES       = all the wxWindows libraries ( and  linker flags on unix )
# WXWIDGETS_CXX_FLAGS        = compiler flags for building wxWindows
# WXWIDGETS_INCLUDE_DIR    = all include path of wxWindows
# WXWIDGETS_DEFINITIONS  = all flags of wxWindows


#
# The flags for the compiler are stored in WXWIDGETS_CXX_FLAGS, we will set those so that
# we know we will always compile wxWidgets programs properly. In case of debugging, we 
# will add the __WXDEBUG__ and WXDEBUG flags as well. For linux, you might want LINUX
# to be defined.
#
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}${WXWIDGETS_CXX_FLAGS}")

ADD_DEFINITIONS( ${WXWIDGETS_DEFINITIONS} ${PROJECT_DEFINITIONS} )

#
# The include dirs
#
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}
                    ${PROJECT_INCLUDE_PATH}
                    ${WXWIDGETS_INCLUDE_DIR})

#
# This is needed to tell CMake what directories contain the libraries we need. This will 
# allow us to just specify the lib files without prepending them with a full path to that
# library
#
LINK_DIRECTORIES( ${WXWIDGETS_LINK_DIRECTORIES} ${PROJECT_LINK_DIRECTORIES})

#
# This is our target. As you can see, we specify WIN32 for windows makefiles. In Linux and 
# other OS'es this flag is ignored. Under windows it will make sure we generate a Win32 file with GUI.
#
ADD_EXECUTABLE(${PROJECT_NAME} WIN32 ${PROJECT_SOURCE_FILES} ${PROJECT_RC_FILE})

#
# Here we specify what libraries are linked to our project
#
TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${WXWIDGETS_LIBRARIES} ${PROJECT_LIBRARIES})

#
# Log results
#
FILE( WRITE "${PROJECT_BINARY_DIR}/usewxw.log" "CMAKE_CXX_FLAGS: ${CMAKE_CXX_FLAGS}\n" )
FILE( APPEND "${PROJECT_BINARY_DIR}/usewxw.log" "Definitions: ${WXWIDGETS_DEFINITIONS} ${PROJECT_DEFINITIONS}\n" )
FILE( APPEND "${PROJECT_BINARY_DIR}/usewxw.log" "INCLUDE_DIRECTORIES: ${PROJECT_SOURCE_DIR} ${PROJECT_INCLUDE_PATH} ${WXWIDGETS_INCLUDE_DIR}\n" )
FILE( APPEND "${PROJECT_BINARY_DIR}/usewxw.log" "LINK_DIRECTORIES: ${WXWIDGETS_LINK_DIRECTORIES} ${PROJECT_LINK_DIRECTORIES}\n" )
FILE( APPEND "${PROJECT_BINARY_DIR}/usewxw.log" "TARGET_LINK_LIBRARIES: ${PROJECT_NAME} ${WXWIDGETS_LIBRARIES} ${PROJECT_LIBRARIES}\n" )
FILE( APPEND "${PROJECT_BINARY_DIR}/usewxw.log" "Sources: ${PROJECT_SOURCE_FILES}\n" )


