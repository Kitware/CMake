# Macro to import the build settings from another project.  Provide as
# an argument the file created by the other project's
# CMAKE_EXPORT_BUILD_SETTINGS command.
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
    STRING(COMPARE NOTEQUAL
           "${CMAKE_BUILD_TOOL}" "${CMAKE_BUILD_SETTING_BUILD_TOOL}"
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
      STRING(COMPARE NOTEQUAL
             "${CMAKE_CXX_COMPILER}" "${CMAKE_BUILD_SETTING_CXX_COMPILER}"
             CMAKE_CXX_COMPILER_MISMATCH)
    ENDIF(CMAKE_BUILD_SETTING_CXX_COMPILER)

    # Check the C build variation flags.
    STRING(COMPARE NOTEQUAL
           "${CMAKE_C_FLAGS_DEBUG}" "${CMAKE_BUILD_SETTING_C_FLAGS_DEBUG}"
           CMAKE_C_FLAGS_DEBUG_MISMATCH)
    STRING(COMPARE NOTEQUAL
           "${CMAKE_C_FLAGS_RELEASE}" "${CMAKE_BUILD_SETTING_C_FLAGS_RELEASE}"
           CMAKE_C_FLAGS_RELEASE_MISMATCH)
    STRING(COMPARE NOTEQUAL
           "${CMAKE_C_FLAGS_MINSIZEREL}" "${CMAKE_BUILD_SETTING_C_FLAGS_MINSIZEREL}"
           CMAKE_C_FLAGS_MINSIZEREL_MISMATCH)
    STRING(COMPARE NOTEQUAL
           "${CMAKE_C_FLAGS_RELWITHDEBINFO}" "${CMAKE_BUILD_SETTING_C_FLAGS_RELWITHDEBINFO}"
           CMAKE_C_FLAGS_RELWITHDEBINFO_MISMATCH)

    # Check the C++ build variation flags.
    STRING(COMPARE NOTEQUAL
           "${CMAKE_CXX_FLAGS_DEBUG}" "${CMAKE_BUILD_SETTING_CXX_FLAGS_DEBUG}"
           CMAKE_CXX_FLAGS_DEBUG_MISMATCH)
    STRING(COMPARE NOTEQUAL
           "${CMAKE_CXX_FLAGS_RELEASE}" "${CMAKE_BUILD_SETTING_CXX_FLAGS_RELEASE}"
           CMAKE_CXX_FLAGS_RELEASE_MISMATCH)
    STRING(COMPARE NOTEQUAL
           "${CMAKE_CXX_FLAGS_MINSIZEREL}" "${CMAKE_BUILD_SETTING_CXX_FLAGS_MINSIZEREL}"
           CMAKE_CXX_FLAGS_MINSIZEREL_MISMATCH)
    STRING(COMPARE NOTEQUAL
           "${CMAKE_CXX_FLAGS_RELWITHDEBINFO}" "${CMAKE_BUILD_SETTING_CXX_FLAGS_RELWITHDEBINFO}"
           CMAKE_CXX_FLAGS_RELWITHDEBINFO_MISMATCH)

    # Check the build type.
    SET(CMAKE_BUILD_TYPE_MISMATCH 0)
    IF(WIN32)
      IF(NOT CMAKE_IMPORT_BUILD_SETTINGS_IMPORTING_FROM_MS_STUDIO)
        STRING(COMPARE NOTEQUAL
               "${CMAKE_BUILD_TYPE}" "${CMAKE_BUILD_SETTING_BUILD_TYPE}"
               CMAKE_BUILD_TYPE_MISMATCH)
      ENDIF(NOT CMAKE_IMPORT_BUILD_SETTINGS_IMPORTING_FROM_MS_STUDIO)
    ENDIF(WIN32)

    # Build tool must match on Windows.
    IF(WIN32)
      IF(CMAKE_BUILD_TOOL_MISMATCH)
        MESSAGE(SEND_ERROR
                "${CMAKE_BUILD_SETTING_PROJECT_NAME} was built using "
                "\"${CMAKE_BUILD_SETTING_BUILD_TOOL}\", but ${PROJECT_NAME} "
                "is using \"${CMAKE_BUILD_TOOL}\".  The build will fail.  "
                "Try selecting a different CMake Generator.")
      ENDIF(CMAKE_BUILD_TOOL_MISMATCH)
    ENDIF(WIN32)

    # Enforce the C++ compiler setting.
    IF(CMAKE_CXX_COMPILER_MISMATCH)
      MESSAGE("Warning: CMake is forcing CMAKE_CXX_COMPILER to "
              "\"${CMAKE_BUILD_SETTING_CXX_COMPILER}\" to match that imported "
              "from ${CMAKE_BUILD_SETTING_PROJECT_NAME}.  This is required "
              "because C++ projects must use the same compiler.  "
              "If this message appears for more than one imported project, "
              "you have conflicting C++ compilers and will have to "
              "re-build one of those projects.")
      SET(CMAKE_CXX_COMPILER ${CMAKE_BUILD_SETTING_CXX_COMPILER}
          CACHE STRING "C++ compiler imported from ${CMAKE_BUILD_SETTING_PROJECT_NAME}." FORCE)
    ENDIF(CMAKE_CXX_COMPILER_MISMATCH)

    # Enforce the build type.
    IF(CMAKE_BUILD_TYPE_MISMATCH)
      MESSAGE("Warning: CMake is forcing CMAKE_BUILD_TYPE to "
              "\"${CMAKE_BUILD_SETTING_BUILD_TYPE}\" to match that imported "
              "from ${CMAKE_BUILD_SETTING_PROJECT_NAME}.  This is required "
              "because projects must use the same compiler settings.  "
              "If this message appears for more than one imported project, "
              "you have conflicting compiler settings and will have to "
              "re-build one of those projects.")
      SET(CMAKE_BUILD_TYPE ${CMAKE_BUILD_SETTING_BUILD_TYPE}
          CACHE STRING "Build type imported from ${CMAKE_BUILD_SETTING_PROJECT_NAME}." FORCE)
    ENDIF(CMAKE_BUILD_TYPE_MISMATCH)

    # Enforce the C build variation flags.

    IF(CMAKE_C_FLAGS_DEBUG_MISMATCH)
      MESSAGE("Warning: CMake is forcing CMAKE_C_FLAGS_DEBUG to "
              "\"${CMAKE_BUILD_SETTING_C_FLAGS_DEBUG}\" to match that imported "
              "from ${CMAKE_BUILD_SETTING_PROJECT_NAME}.  "
              "If this message appears for more than one imported project, "
              "you have conflicting options and will have to "
              "re-build one of those projects.")
      SET(CMAKE_C_FLAGS_DEBUG ${CMAKE_BUILD_SETTING_C_FLAGS_DEBUG}
          CACHE STRING "C DEBUG flags imported from ${CMAKE_BUILD_SETTING_PROJECT_NAME}." FORCE)
    ENDIF(CMAKE_C_FLAGS_DEBUG_MISMATCH)

    IF(CMAKE_C_FLAGS_RELEASE_MISMATCH)
      MESSAGE("Warning: CMake is forcing CMAKE_C_FLAGS_RELEASE to "
              "\"${CMAKE_BUILD_SETTING_C_FLAGS_RELEASE}\" to match that imported "
              "from ${CMAKE_BUILD_SETTING_PROJECT_NAME}.  "
              "If this message appears for more than one imported project, "
              "you have conflicting options and will have to "
              "re-build one of those projects.")
      SET(CMAKE_C_FLAGS_RELEASE ${CMAKE_BUILD_SETTING_C_FLAGS_RELEASE}
          CACHE STRING "C RELEASE flags imported from ${CMAKE_BUILD_SETTING_PROJECT_NAME}." FORCE)
    ENDIF(CMAKE_C_FLAGS_RELEASE_MISMATCH)

    IF(CMAKE_C_FLAGS_MINSIZEREL_MISMATCH)
      MESSAGE("Warning: CMake is forcing CMAKE_C_FLAGS_MINSIZEREL to "
              "\"${CMAKE_BUILD_SETTING_C_FLAGS_MINSIZEREL}\" to match that imported "
              "from ${CMAKE_BUILD_SETTING_PROJECT_NAME}.  "
              "If this message appears for more than one imported project, "
              "you have conflicting options and will have to "
              "re-build one of those projects.")
      SET(CMAKE_C_FLAGS_MINSIZEREL ${CMAKE_BUILD_SETTING_C_FLAGS_MINSIZEREL}
          CACHE STRING "C MINSIZEREL flags imported from ${CMAKE_BUILD_SETTING_PROJECT_NAME}." FORCE)
    ENDIF(CMAKE_C_FLAGS_MINSIZEREL_MISMATCH)

    IF(CMAKE_C_FLAGS_RELWITHDEBINFO_MISMATCH)
      MESSAGE("Warning: CMake is forcing CMAKE_C_FLAGS_RELWITHDEBINFO to "
              "\"${CMAKE_BUILD_SETTING_C_FLAGS_RELWITHDEBINFO}\" to match that imported "
              "from ${CMAKE_BUILD_SETTING_PROJECT_NAME}.  "
              "If this message appears for more than one imported project, "
              "you have conflicting options and will have to "
              "re-build one of those projects.")
      SET(CMAKE_C_FLAGS_RELWITHDEBINFO ${CMAKE_BUILD_SETTING_C_FLAGS_RELWITHDEBINFO}
          CACHE STRING "C RELWITHDEBINFO flags imported from ${CMAKE_BUILD_SETTING_PROJECT_NAME}." FORCE)
    ENDIF(CMAKE_C_FLAGS_RELWITHDEBINFO_MISMATCH)

    # Enforce the C++ build variation flags.

    IF(CMAKE_CXX_FLAGS_DEBUG_MISMATCH)
      MESSAGE("Warning: CMake is forcing CMAKE_CXX_FLAGS_DEBUG to "
              "\"${CMAKE_BUILD_SETTING_CXX_FLAGS_DEBUG}\" to match that imported "
              "from ${CMAKE_BUILD_SETTING_PROJECT_NAME}.  "
              "If this message appears for more than one imported project, "
              "you have conflicting options and will have to "
              "re-build one of those projects.")
      SET(CMAKE_CXX_FLAGS_DEBUG ${CMAKE_BUILD_SETTING_CXX_FLAGS_DEBUG}
          CACHE STRING "C++ DEBUG flags imported from ${CMAKE_BUILD_SETTING_PROJECT_NAME}." FORCE)
    ENDIF(CMAKE_CXX_FLAGS_DEBUG_MISMATCH)

    IF(CMAKE_CXX_FLAGS_RELEASE_MISMATCH)
      MESSAGE("Warning: CMake is forcing CMAKE_CXX_FLAGS_RELEASE to "
              "\"${CMAKE_BUILD_SETTING_CXX_FLAGS_RELEASE}\" to match that imported "
              "from ${CMAKE_BUILD_SETTING_PROJECT_NAME}.  "
              "If this message appears for more than one imported project, "
              "you have conflicting options and will have to "
              "re-build one of those projects.")
      SET(CMAKE_CXX_FLAGS_RELEASE ${CMAKE_BUILD_SETTING_CXX_FLAGS_RELEASE}
          CACHE STRING "C++ RELEASE flags imported from ${CMAKE_BUILD_SETTING_PROJECT_NAME}." FORCE)
    ENDIF(CMAKE_CXX_FLAGS_RELEASE_MISMATCH)

    IF(CMAKE_CXX_FLAGS_MINSIZEREL_MISMATCH)
      MESSAGE("Warning: CMake is forcing CMAKE_CXX_FLAGS_MINSIZEREL to "
              "\"${CMAKE_BUILD_SETTING_CXX_FLAGS_MINSIZEREL}\" to match that imported "
              "from ${CMAKE_BUILD_SETTING_PROJECT_NAME}.  "
              "If this message appears for more than one imported project, "
              "you have conflicting options and will have to "
              "re-build one of those projects.")
      SET(CMAKE_CXX_FLAGS_MINSIZEREL ${CMAKE_BUILD_SETTING_CXX_FLAGS_MINSIZEREL}
          CACHE STRING "C++ MINSIZEREL flags imported from ${CMAKE_BUILD_SETTING_PROJECT_NAME}." FORCE)
    ENDIF(CMAKE_CXX_FLAGS_MINSIZEREL_MISMATCH)

    IF(CMAKE_CXX_FLAGS_RELWITHDEBINFO_MISMATCH)
      MESSAGE("Warning: CMake is forcing CMAKE_CXX_FLAGS_RELWITHDEBINFO to "
              "\"${CMAKE_BUILD_SETTING_CXX_FLAGS_RELWITHDEBINFO}\" to match that imported "
              "from ${CMAKE_BUILD_SETTING_PROJECT_NAME}.  "
              "If this message appears for more than one imported project, "
              "you have conflicting options and will have to "
              "re-build one of those projects.")
      SET(CMAKE_CXX_FLAGS_RELWITHDEBINFO ${CMAKE_BUILD_SETTING_CXX_FLAGS_RELWITHDEBINFO}
          CACHE STRING "C++ RELWITHDEBINFO flags imported from ${CMAKE_BUILD_SETTING_PROJECT_NAME}." FORCE)
    ENDIF(CMAKE_CXX_FLAGS_RELWITHDEBINFO_MISMATCH)

  ELSE(${SETTINGS_FILE} MATCHES ".+")
    MESSAGE(SEND_ERROR "CMAKE_IMPORT_BUILD_SETTINGS called with no argument.")
  ENDIF(${SETTINGS_FILE} MATCHES ".+")
ENDMACRO(CMAKE_IMPORT_BUILD_SETTINGS)
