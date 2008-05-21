# - import build settings from another project
#  CMAKE_IMPORT_BUILD_SETTINGS(SETTINGS_FILE) 
# macro defined to import the build settings from another project.  
# SETTINGS_FILE is a file created by the other project's call to the
# CMAKE_EXPORT_BUILD_SETTINGS macro, see CMakeExportBuildSettings.
MACRO(CMAKE_IMPORT_BUILD_SETTINGS SETTINGS_FILE)
  IF(${SETTINGS_FILE} MATCHES ".+")
    # Load the settings.
    INCLUDE(${SETTINGS_FILE})

    # Check the CMake version that stored the settings.
    IF(${CMAKE_BUILD_SETTING_CMAKE_MAJOR_VERSION}.${CMAKE_BUILD_SETTING_CMAKE_MINOR_VERSION}
       GREATER ${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION})
      MESSAGE(SEND_ERROR
              "${CMAKE_BUILD_SETTING_PROJECT_NAME} was built using CMake "
              "${CMAKE_BUILD_SETTING_CMAKE_MAJOR_VERSION}.${CMAKE_BUILD_SETTING_CMAKE_MINOR_VERSION}, "
              "but this is CMake${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION}.  "
              "Please upgrade CMake to a more recent version.")
    ENDIF(${CMAKE_BUILD_SETTING_CMAKE_MAJOR_VERSION}.${CMAKE_BUILD_SETTING_CMAKE_MINOR_VERSION}
       GREATER ${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION})

    # Check the build tool.
    SET(CMAKE_BUILD_TOOL1 "")
    IF(CMAKE_BUILD_TOOL)
      GET_FILENAME_COMPONENT (CMAKE_BUILD_TOOL1 ${CMAKE_BUILD_TOOL} NAME_WE)
      STRING(TOLOWER ${CMAKE_BUILD_TOOL1} CMAKE_BUILD_TOOL1)
    ENDIF(CMAKE_BUILD_TOOL)
    SET(CMAKE_BUILD_TOOL2 "")
    IF(CMAKE_BUILD_SETTING_BUILD_TOOL)
      GET_FILENAME_COMPONENT (CMAKE_BUILD_TOOL2 ${CMAKE_BUILD_SETTING_BUILD_TOOL} NAME_WE)
      STRING(TOLOWER ${CMAKE_BUILD_TOOL2} CMAKE_BUILD_TOOL2)
    ENDIF(CMAKE_BUILD_SETTING_BUILD_TOOL)
    STRING(COMPARE NOTEQUAL "x${CMAKE_BUILD_TOOL1}" "x${CMAKE_BUILD_TOOL2}"
           CMAKE_BUILD_TOOL_MISMATCH)

    IF(CMAKE_BUILD_SETTING_BUILD_TOOL MATCHES "^msdev$")
      SET(CMAKE_IMPORT_BUILD_SETTINGS_IMPORTING_FROM_MS_STUDIO 1)
    ENDIF(CMAKE_BUILD_SETTING_BUILD_TOOL MATCHES "^msdev$")
    IF(CMAKE_BUILD_SETTING_BUILD_TOOL MATCHES "^devenv$")
      SET(CMAKE_IMPORT_BUILD_SETTINGS_IMPORTING_FROM_MS_STUDIO 1)
    ENDIF(CMAKE_BUILD_SETTING_BUILD_TOOL MATCHES "^devenv$")

    # Check the C++ compiler setting.  If it is empty, the imported
    # project is not a C++ project, and doesn't need a matching compiler.
    IF(CMAKE_BUILD_SETTING_CXX_COMPILER)
      IF(WIN32)
        STRING(TOLOWER "x${CMAKE_CXX_COMPILER}" COMPARE_CXX_LOCAL)
        STRING(TOLOWER "x${CMAKE_BUILD_SETTING_CXX_COMPILER}" COMPARE_CXX_REMOTE)
        STRING(COMPARE NOTEQUAL "${COMPARE_CXX_LOCAL}" "${COMPARE_CXX_REMOTE}"
          CMAKE_CXX_COMPILER_MISMATCH)
      ELSE(WIN32)
        STRING(COMPARE NOTEQUAL
          "x${CMAKE_CXX_COMPILER}" "x${CMAKE_BUILD_SETTING_CXX_COMPILER}"
          CMAKE_CXX_COMPILER_MISMATCH)
      ENDIF(WIN32)
    ENDIF(CMAKE_BUILD_SETTING_CXX_COMPILER)

    # Check the C build variation flags.
    STRING(COMPARE NOTEQUAL
           "x${CMAKE_C_FLAGS_DEBUG}" "x${CMAKE_BUILD_SETTING_C_FLAGS_DEBUG}"
           CMAKE_C_FLAGS_DEBUG_MISMATCH)
    STRING(COMPARE NOTEQUAL
           "x${CMAKE_C_FLAGS_RELEASE}" "x${CMAKE_BUILD_SETTING_C_FLAGS_RELEASE}"
           CMAKE_C_FLAGS_RELEASE_MISMATCH)
    STRING(COMPARE NOTEQUAL
           "x${CMAKE_C_FLAGS_MINSIZEREL}" "x${CMAKE_BUILD_SETTING_C_FLAGS_MINSIZEREL}"
           CMAKE_C_FLAGS_MINSIZEREL_MISMATCH)
    STRING(COMPARE NOTEQUAL
           "x${CMAKE_C_FLAGS_RELWITHDEBINFO}" "x${CMAKE_BUILD_SETTING_C_FLAGS_RELWITHDEBINFO}"
           CMAKE_C_FLAGS_RELWITHDEBINFO_MISMATCH)

    # Check the C++ build variation flags.
    STRING(COMPARE NOTEQUAL
           "x${CMAKE_CXX_FLAGS_DEBUG}" "x${CMAKE_BUILD_SETTING_CXX_FLAGS_DEBUG}"
           CMAKE_CXX_FLAGS_DEBUG_MISMATCH)
    STRING(COMPARE NOTEQUAL
           "x${CMAKE_CXX_FLAGS_RELEASE}" "x${CMAKE_BUILD_SETTING_CXX_FLAGS_RELEASE}"
           CMAKE_CXX_FLAGS_RELEASE_MISMATCH)
    STRING(COMPARE NOTEQUAL
           "x${CMAKE_CXX_FLAGS_MINSIZEREL}" "x${CMAKE_BUILD_SETTING_CXX_FLAGS_MINSIZEREL}"
           CMAKE_CXX_FLAGS_MINSIZEREL_MISMATCH)
    STRING(COMPARE NOTEQUAL
           "x${CMAKE_CXX_FLAGS_RELWITHDEBINFO}" "x${CMAKE_BUILD_SETTING_CXX_FLAGS_RELWITHDEBINFO}"
           CMAKE_CXX_FLAGS_RELWITHDEBINFO_MISMATCH)

    # Check the build type.
    SET(CMAKE_BUILD_TYPE_MISMATCH 0)
    IF(WIN32)
      IF(NOT CMAKE_IMPORT_BUILD_SETTINGS_IMPORTING_FROM_MS_STUDIO)
        STRING(COMPARE NOTEQUAL
               "x${CMAKE_BUILD_TYPE}" "x${CMAKE_BUILD_SETTING_BUILD_TYPE}"
               CMAKE_BUILD_TYPE_MISMATCH)
      ENDIF(NOT CMAKE_IMPORT_BUILD_SETTINGS_IMPORTING_FROM_MS_STUDIO)
    ENDIF(WIN32)

    # Build tool must match on Windows.
    IF(WIN32)
      IF(CMAKE_BUILD_TOOL_MISMATCH AND NOT CMAKE_OVERRIDE_COMPILER_MISMATCH)
        MESSAGE(SEND_ERROR
                "This project, ${PROJECT_NAME}, depends on the project "
                "${CMAKE_BUILD_SETTING_PROJECT_NAME}. Unfortunately "
                "${CMAKE_BUILD_SETTING_PROJECT_NAME} was built using "
                "\"${CMAKE_BUILD_TOOL2}\", but you are trying to build "
                "${PROJECT_NAME} using \"${CMAKE_BUILD_TOOL1}\"."
                "In order for this build to succeed, both projects must be "
                "built with the same Generator. To change the Generator you "
                "are using for the project ${PROJECT_NAME}, you must "
                "delete the cache, and then rerun cmake and this "
                "time select the same Generator that was used to build "
                "${CMAKE_BUILD_SETTING_PROJECT_NAME}. If "
                "${CMAKE_BUILD_SETTING_PROJECT_NAME} was built using a "
                "generator that you do not have (for example it was built "
                "with Visual Studio 6 and you only have 7) then you will "
                "need to select a different version of "
                "${CMAKE_BUILD_SETTING_PROJECT_NAME} or rebuild "
                "${CMAKE_BUILD_SETTING_PROJECT_NAME} with the correct "
                "generator. ")
      ENDIF(CMAKE_BUILD_TOOL_MISMATCH AND NOT CMAKE_OVERRIDE_COMPILER_MISMATCH)
    ENDIF(WIN32)

    # Enforce the C++ compiler setting.
    # CMAKE_OVERRIDE_COMPILER_MISMATCH allow advanced user to override cmake detection of
    # compiler mismatch between imported projects. Typical case on UNIX could be:
    # 1. Compile a project with g++-3.3 while the imported project was configured
    # using the 'c++' alias (which at the time meant g++-3.3)
    # 2. This variable also becomes handy when the project your are importing has been
    # compiled with a compiler you do not have access to, but offer a compatible ABI with
    # yours.
    # WARNING: Do not use this variable with C++ compiler with incompatible ABI
    IF(CMAKE_CXX_COMPILER_MISMATCH AND NOT CMAKE_OVERRIDE_COMPILER_MISMATCH)
      MESSAGE("Warning: CMake is forcing CMAKE_CXX_COMPILER to "
              "\"${CMAKE_BUILD_SETTING_CXX_COMPILER}\" to match that imported "
              "from ${CMAKE_BUILD_SETTING_PROJECT_NAME}.  This is required "
              "because C++ projects must use the same compiler.  "
              "If this message appears for more than one imported project, "
              "you have conflicting C++ compilers and will have to "
              "re-build one of those projects. Was set to ${CMAKE_CXX_COMPILER}")
      SET(CMAKE_CXX_COMPILER ${CMAKE_BUILD_SETTING_CXX_COMPILER}
          CACHE STRING "C++ compiler imported from ${CMAKE_BUILD_SETTING_PROJECT_NAME}." FORCE)
    ENDIF(CMAKE_CXX_COMPILER_MISMATCH AND NOT CMAKE_OVERRIDE_COMPILER_MISMATCH)

    # Enforce the build type.
    IF(CMAKE_BUILD_TYPE_MISMATCH AND NOT CMAKE_OVERRIDE_COMPILER_MISMATCH)
      MESSAGE("Warning: CMake is forcing CMAKE_BUILD_TYPE to "
              "\"${CMAKE_BUILD_SETTING_BUILD_TYPE}\" to match that imported "
              "from ${CMAKE_BUILD_SETTING_PROJECT_NAME}.  This is required "
              "because projects must use the same compiler settings.  "
              "If this message appears for more than one imported project, "
              "you have conflicting compiler settings and will have to "
              "re-build one of those projects.")
      SET(CMAKE_BUILD_TYPE ${CMAKE_BUILD_SETTING_BUILD_TYPE}
          CACHE STRING "Build type imported from ${CMAKE_BUILD_SETTING_PROJECT_NAME}." FORCE)
    ENDIF(CMAKE_BUILD_TYPE_MISMATCH AND NOT CMAKE_OVERRIDE_COMPILER_MISMATCH)

    # Enforce the C build variation flags.

    IF(CMAKE_C_FLAGS_DEBUG_MISMATCH AND NOT CMAKE_OVERRIDE_COMPILER_MISMATCH)
      MESSAGE("Warning: CMake is forcing CMAKE_C_FLAGS_DEBUG to "
              "\"${CMAKE_BUILD_SETTING_C_FLAGS_DEBUG}\" to match that imported "
              "from ${CMAKE_BUILD_SETTING_PROJECT_NAME}.  "
              "If this message appears for more than one imported project, "
              "you have conflicting options and will have to "
              "re-build one of those projects.")
      SET(CMAKE_C_FLAGS_DEBUG ${CMAKE_BUILD_SETTING_C_FLAGS_DEBUG}
          CACHE STRING "C DEBUG flags imported from ${CMAKE_BUILD_SETTING_PROJECT_NAME}." FORCE)
    ENDIF(CMAKE_C_FLAGS_DEBUG_MISMATCH AND NOT CMAKE_OVERRIDE_COMPILER_MISMATCH)

    IF(CMAKE_C_FLAGS_RELEASE_MISMATCH AND NOT CMAKE_OVERRIDE_COMPILER_MISMATCH)
      MESSAGE("Warning: CMake is forcing CMAKE_C_FLAGS_RELEASE to "
              "\"${CMAKE_BUILD_SETTING_C_FLAGS_RELEASE}\" to match that imported "
              "from ${CMAKE_BUILD_SETTING_PROJECT_NAME}.  "
              "If this message appears for more than one imported project, "
              "you have conflicting options and will have to "
              "re-build one of those projects.")
      SET(CMAKE_C_FLAGS_RELEASE ${CMAKE_BUILD_SETTING_C_FLAGS_RELEASE}
          CACHE STRING "C RELEASE flags imported from ${CMAKE_BUILD_SETTING_PROJECT_NAME}." FORCE)
    ENDIF(CMAKE_C_FLAGS_RELEASE_MISMATCH AND NOT CMAKE_OVERRIDE_COMPILER_MISMATCH)

    IF(CMAKE_C_FLAGS_MINSIZEREL_MISMATCH AND NOT CMAKE_OVERRIDE_COMPILER_MISMATCH)
      MESSAGE("Warning: CMake is forcing CMAKE_C_FLAGS_MINSIZEREL to "
              "\"${CMAKE_BUILD_SETTING_C_FLAGS_MINSIZEREL}\" to match that imported "
              "from ${CMAKE_BUILD_SETTING_PROJECT_NAME}.  "
              "If this message appears for more than one imported project, "
              "you have conflicting options and will have to "
              "re-build one of those projects.")
      SET(CMAKE_C_FLAGS_MINSIZEREL ${CMAKE_BUILD_SETTING_C_FLAGS_MINSIZEREL}
          CACHE STRING "C MINSIZEREL flags imported from ${CMAKE_BUILD_SETTING_PROJECT_NAME}." FORCE)
    ENDIF(CMAKE_C_FLAGS_MINSIZEREL_MISMATCH AND NOT CMAKE_OVERRIDE_COMPILER_MISMATCH)

    IF(CMAKE_C_FLAGS_RELWITHDEBINFO_MISMATCH AND NOT CMAKE_OVERRIDE_COMPILER_MISMATCH)
      MESSAGE("Warning: CMake is forcing CMAKE_C_FLAGS_RELWITHDEBINFO to "
              "\"${CMAKE_BUILD_SETTING_C_FLAGS_RELWITHDEBINFO}\" to match that imported "
              "from ${CMAKE_BUILD_SETTING_PROJECT_NAME}.  "
              "If this message appears for more than one imported project, "
              "you have conflicting options and will have to "
              "re-build one of those projects.")
      SET(CMAKE_C_FLAGS_RELWITHDEBINFO ${CMAKE_BUILD_SETTING_C_FLAGS_RELWITHDEBINFO}
          CACHE STRING "C RELWITHDEBINFO flags imported from ${CMAKE_BUILD_SETTING_PROJECT_NAME}." FORCE)
    ENDIF(CMAKE_C_FLAGS_RELWITHDEBINFO_MISMATCH AND NOT CMAKE_OVERRIDE_COMPILER_MISMATCH)

    # Enforce the C++ build variation flags.

    IF(CMAKE_CXX_FLAGS_DEBUG_MISMATCH AND NOT CMAKE_OVERRIDE_COMPILER_MISMATCH)
      MESSAGE("Warning: CMake is forcing CMAKE_CXX_FLAGS_DEBUG to "
              "\"${CMAKE_BUILD_SETTING_CXX_FLAGS_DEBUG}\" to match that imported "
              "from ${CMAKE_BUILD_SETTING_PROJECT_NAME}.  "
              "If this message appears for more than one imported project, "
              "you have conflicting options and will have to "
              "re-build one of those projects.")
      SET(CMAKE_CXX_FLAGS_DEBUG ${CMAKE_BUILD_SETTING_CXX_FLAGS_DEBUG}
          CACHE STRING "C++ DEBUG flags imported from ${CMAKE_BUILD_SETTING_PROJECT_NAME}." FORCE)
    ENDIF(CMAKE_CXX_FLAGS_DEBUG_MISMATCH AND NOT CMAKE_OVERRIDE_COMPILER_MISMATCH)

    IF(CMAKE_CXX_FLAGS_RELEASE_MISMATCH AND NOT CMAKE_OVERRIDE_COMPILER_MISMATCH)
      MESSAGE("Warning: CMake is forcing CMAKE_CXX_FLAGS_RELEASE to "
              "\"${CMAKE_BUILD_SETTING_CXX_FLAGS_RELEASE}\" to match that imported "
              "from ${CMAKE_BUILD_SETTING_PROJECT_NAME}.  "
              "If this message appears for more than one imported project, "
              "you have conflicting options and will have to "
              "re-build one of those projects.")
      SET(CMAKE_CXX_FLAGS_RELEASE ${CMAKE_BUILD_SETTING_CXX_FLAGS_RELEASE}
          CACHE STRING "C++ RELEASE flags imported from ${CMAKE_BUILD_SETTING_PROJECT_NAME}." FORCE)
    ENDIF(CMAKE_CXX_FLAGS_RELEASE_MISMATCH AND NOT CMAKE_OVERRIDE_COMPILER_MISMATCH)

    IF(CMAKE_CXX_FLAGS_MINSIZEREL_MISMATCH AND NOT CMAKE_OVERRIDE_COMPILER_MISMATCH)
      MESSAGE("Warning: CMake is forcing CMAKE_CXX_FLAGS_MINSIZEREL to "
              "\"${CMAKE_BUILD_SETTING_CXX_FLAGS_MINSIZEREL}\" to match that imported "
              "from ${CMAKE_BUILD_SETTING_PROJECT_NAME}.  "
              "If this message appears for more than one imported project, "
              "you have conflicting options and will have to "
              "re-build one of those projects.")
      SET(CMAKE_CXX_FLAGS_MINSIZEREL ${CMAKE_BUILD_SETTING_CXX_FLAGS_MINSIZEREL}
          CACHE STRING "C++ MINSIZEREL flags imported from ${CMAKE_BUILD_SETTING_PROJECT_NAME}." FORCE)
    ENDIF(CMAKE_CXX_FLAGS_MINSIZEREL_MISMATCH AND NOT CMAKE_OVERRIDE_COMPILER_MISMATCH)

    IF(CMAKE_CXX_FLAGS_RELWITHDEBINFO_MISMATCH AND NOT CMAKE_OVERRIDE_COMPILER_MISMATCH)
      MESSAGE("Warning: CMake is forcing CMAKE_CXX_FLAGS_RELWITHDEBINFO to "
              "\"${CMAKE_BUILD_SETTING_CXX_FLAGS_RELWITHDEBINFO}\" to match that imported "
              "from ${CMAKE_BUILD_SETTING_PROJECT_NAME}.  "
              "If this message appears for more than one imported project, "
              "you have conflicting options and will have to "
              "re-build one of those projects.")
      SET(CMAKE_CXX_FLAGS_RELWITHDEBINFO ${CMAKE_BUILD_SETTING_CXX_FLAGS_RELWITHDEBINFO}
          CACHE STRING "C++ RELWITHDEBINFO flags imported from ${CMAKE_BUILD_SETTING_PROJECT_NAME}." FORCE)
    ENDIF(CMAKE_CXX_FLAGS_RELWITHDEBINFO_MISMATCH AND NOT CMAKE_OVERRIDE_COMPILER_MISMATCH)

  ELSE(${SETTINGS_FILE} MATCHES ".+")
    MESSAGE(SEND_ERROR "CMAKE_IMPORT_BUILD_SETTINGS called with no argument.")
  ENDIF(${SETTINGS_FILE} MATCHES ".+")
ENDMACRO(CMAKE_IMPORT_BUILD_SETTINGS)
