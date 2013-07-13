
function(QUERY_QMAKE VAR RESULT)
  execute_process(COMMAND "${QT_QMAKE_EXECUTABLE}" -query ${VAR}
    RESULT_VARIABLE return_code
    OUTPUT_VARIABLE output
    OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_STRIP_TRAILING_WHITESPACE)
  if(NOT return_code)
    file(TO_CMAKE_PATH "${output}" output)
    set(${RESULT} ${output} PARENT_SCOPE)
  endif()
endfunction()

function(get_version_components VERSION RESULT_MAJOR RESULT_MINOR RESULT_PATCH)
  string(REGEX REPLACE "^([0-9]+)\\.[0-9]+\\.[0-9]+.*" "\\1" QT_VERSION_MAJOR "${QTVERSION}")
  string(REGEX REPLACE "^[0-9]+\\.([0-9]+)\\.[0-9]+.*" "\\1" QT_VERSION_MINOR "${QTVERSION}")
  string(REGEX REPLACE "^[0-9]+\\.[0-9]+\\.([0-9]+).*" "\\1" QT_VERSION_PATCH "${QTVERSION}")

  set(${RESULT_MAJOR} ${QT_VERSION_MAJOR} PARENT_SCOPE)
  set(${RESULT_MINOR} ${QT_VERSION_MINOR} PARENT_SCOPE)
  set(${RESULT_PATCH} ${QT_VERSION_PATCH} PARENT_SCOPE)
endfunction()

function(test_find_qmake QMAKE_NAMES QMAKE_RESULT VERSION_RESULT)
  list(LENGTH QMAKE_NAMES QMAKE_NAMES_LEN)
  if(${QMAKE_NAMES_LEN} EQUAL 0)
    return()
  endif()
  list(GET QMAKE_NAMES 0 QMAKE_NAME)

  get_filename_component(qt_install_version "[HKEY_CURRENT_USER\\Software\\trolltech\\Versions;DefaultQtVersion]" NAME)

  find_program(QT_QMAKE_EXECUTABLE NAMES ${QMAKE_NAME}
    PATHS
      ENV QTDIR
      "[HKEY_CURRENT_USER\\Software\\Trolltech\\Versions\\${qt_install_version};InstallDir]"
    PATH_SUFFIXES bin
    DOC "The qmake executable for the Qt installation to use"
  )

  set(major 0)
  if (QT_QMAKE_EXECUTABLE)
    query_qmake(QT_VERSION QTVERSION)
    get_version_components("${QTVERSION}" major minor patch)
  endif()

  if (NOT QT_QMAKE_EXECUTABLE OR NOT "${major}" EQUAL 4)
    set(curr_qmake "${QT_QMAKE_EXECUTABLE}")
    set(curr_qt_version "${QTVERSION}")

    set(QT_QMAKE_EXECUTABLE NOTFOUND CACHE FILEPATH "" FORCE)
    list(REMOVE_AT QMAKE_NAMES 0)
    test_find_qmake("${QMAKE_NAMES}" QMAKE QTVERSION)

    get_version_components("${QTVERSION}" major minor patch)
    if (NOT ${major} EQUAL 4)
      # Restore possibly found qmake and it's version; these are used later
      # in error message if incorrect version is found
      set(QT_QMAKE_EXECUTABLE "${curr_qmake}" CACHE FILEPATH "" FORCE)
      set(QTVERSION "${curr_qt_version}")
    endif()

  endif()


  set(${QMAKE_RESULT} "${QT_QMAKE_EXECUTABLE}" PARENT_SCOPE)
  set(${VERSION_RESULT} "${QTVERSION}" PARENT_SCOPE)
endfunction()

set(QMAKE_NAMES qmake qmake4 qmake-qt4 qmake-mac)
test_find_qmake("${QMAKE_NAMES}" QMAKE_EXECUTABLE TESTQTVERSION)

message("QMAKE_EXECUTABLE : ${QMAKE_EXECUTABLE}")
message("TESTQTVERSION : ${TESTQTVERSION}")

find_package(Qt4 REQUIRED)

set(CMAKE_WARN_DEPRECATED 1)

add_library(foo SHARED empty.cpp)
qt4_use_modules(foo LINK_PRIVATE Core)
