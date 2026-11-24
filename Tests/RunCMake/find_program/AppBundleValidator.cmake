function(make_bundle dir name)
  set(exe "${CMAKE_CURRENT_BINARY_DIR}/${dir}/${name}.app/Contents/MacOS/${name}")
  file(WRITE "${exe}" "#!/bin/sh\n")
  file(CHMOD "${exe}" PERMISSIONS OWNER_READ OWNER_EXECUTE)
endfunction()
set(CMAKE_FIND_APPBUNDLE ONLY)

# A bundle rejected by the validator does not end the search.
make_bundle(A MyApp)
make_bundle(B MyApp)
function(check_b result path)
  if(NOT path MATCHES "/B/MyApp\\.app/Contents/MacOS/MyApp$")
    set(${result} FALSE PARENT_SCOPE)
  endif()
endfunction()
find_program(PROG NAMES MyApp NO_DEFAULT_PATH VALIDATOR check_b
  PATHS "${CMAKE_CURRENT_BINARY_DIR}/A" "${CMAKE_CURRENT_BINARY_DIR}/B")
message(STATUS "PROG='${PROG}'")

# A <name>.app that is not a directory ends the search for that name,
# but not for the next name, which is not in the first directory.
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/File/MyApp.app" "")
make_bundle(FileNext MyApp)
make_bundle(FileNext Other)
find_program(PROG_FILE NAMES MyApp Other NO_DEFAULT_PATH
  PATHS "${CMAKE_CURRENT_BINARY_DIR}/File" "${CMAKE_CURRENT_BINARY_DIR}/FileNext")
message(STATUS "PROG_FILE='${PROG_FILE}'")

# A <name>.app with no executable ends the search for that name.
file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/NoExe/MyApp.app/Contents/MacOS")
make_bundle(NoExeNext MyApp)
make_bundle(NoExeNext Other)
find_program(PROG_NOEXE NAMES MyApp Other NO_DEFAULT_PATH
  PATHS "${CMAKE_CURRENT_BINARY_DIR}/NoExe" "${CMAKE_CURRENT_BINARY_DIR}/NoExeNext")
message(STATUS "PROG_NOEXE='${PROG_NOEXE}'")
