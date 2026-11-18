if(actual_stdout MATCHES "has no symbols|table of contents is empty")
  string(APPEND RunCMake_TEST_FAILED
    "Archiver warned about missing symbols:\n${actual_stdout}\n")
endif()

foreach(tgt IN ITEMS NoMocStatic NoMocObject NoMocShared NoMocExe)
  file(GLOB mocs_compilation
    "${RunCMake_TEST_BINARY_DIR}/${tgt}_autogen/mocs_compilation*.cpp")
  if(NOT mocs_compilation)
    string(APPEND RunCMake_TEST_FAILED "No mocs_compilation.cpp for ${tgt}\n")
  endif()
  foreach(file IN LISTS mocs_compilation)
    file(READ "${file}" content)
    if(NOT content MATCHES "cmake_automoc_silence_linker_warning")
      string(APPEND RunCMake_TEST_FAILED "Placeholder symbol missing in:\n  ${file}\n")
    endif()
  endforeach()
endforeach()
