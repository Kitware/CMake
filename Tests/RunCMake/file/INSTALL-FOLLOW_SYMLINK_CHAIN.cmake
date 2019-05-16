file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/dest1")

file(TOUCH "${CMAKE_BINARY_DIR}/file1.txt")
file(CREATE_LINK file1.txt "${CMAKE_BINARY_DIR}/file1.txt.sym" SYMBOLIC)
file(TOUCH "${CMAKE_BINARY_DIR}/dest1/file1.txt.sym")

file(TOUCH "${CMAKE_BINARY_DIR}/file2.txt")
file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/file2")
file(CREATE_LINK ../file2.txt "${CMAKE_BINARY_DIR}/file2/file2.txt.sym" SYMBOLIC)

file(TOUCH "${CMAKE_BINARY_DIR}/file3.txt")
file(CREATE_LINK "${CMAKE_BINARY_DIR}/file3.txt" "${CMAKE_BINARY_DIR}/file3.txt.sym" SYMBOLIC)

file(TOUCH "${CMAKE_BINARY_DIR}/file4.txt")
file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/file4")
file(CREATE_LINK "${CMAKE_BINARY_DIR}/file4.txt" "${CMAKE_BINARY_DIR}/file4/file4.txt.sym" SYMBOLIC)

file(TOUCH "${CMAKE_BINARY_DIR}/file5.txt")

file(TOUCH "${CMAKE_BINARY_DIR}/file6.txt")
file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/file6/file6")
file(CREATE_LINK file6.txt "${CMAKE_BINARY_DIR}/file6.txt.sym.1" SYMBOLIC)
file(CREATE_LINK ../file6.txt.sym.1 "${CMAKE_BINARY_DIR}/file6/file6.txt.sym.2" SYMBOLIC)
file(CREATE_LINK "${CMAKE_BINARY_DIR}/file6/file6.txt.sym.2" "${CMAKE_BINARY_DIR}/file6/file6/file6.txt.sym.3" SYMBOLIC)
file(CREATE_LINK file6.txt.sym.3 "${CMAKE_BINARY_DIR}/file6/file6/file6.txt.sym.4" SYMBOLIC)

file(TOUCH "${CMAKE_BINARY_DIR}/file7.txt")
file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/file7")

file(TOUCH "${CMAKE_BINARY_DIR}/file8.txt")
file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/file8")
file(CREATE_LINK "${CMAKE_BINARY_DIR}/file8/../file8.txt" "${CMAKE_BINARY_DIR}/file8/file8.txt.sym" SYMBOLIC)

file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/file9")
file(TOUCH "${CMAKE_BINARY_DIR}/file9/file9.txt")
file(CREATE_LINK "${CMAKE_BINARY_DIR}/file9" "${CMAKE_BINARY_DIR}/file9.sym" SYMBOLIC)

file(TOUCH "${CMAKE_BINARY_DIR}/file10.txt")
file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/file10")
file(CREATE_LINK "." "${CMAKE_BINARY_DIR}/file10/file10" SYMBOLIC)
file(CREATE_LINK "${CMAKE_BINARY_DIR}/file10/file10/../file10.txt" "${CMAKE_BINARY_DIR}/file10/file10.txt.sym" SYMBOLIC)

file(INSTALL
  "${CMAKE_BINARY_DIR}/file1.txt.sym"
  DESTINATION "${CMAKE_BINARY_DIR}/dest1"
  FOLLOW_SYMLINK_CHAIN
  )

file(INSTALL
  "${CMAKE_BINARY_DIR}/file1.txt.sym"
  "${CMAKE_BINARY_DIR}/file2/file2.txt.sym"
  "${CMAKE_BINARY_DIR}/file3.txt.sym"
  "${CMAKE_BINARY_DIR}/file4/file4.txt.sym"
  "${CMAKE_BINARY_DIR}/file5.txt"
  "${CMAKE_BINARY_DIR}/file6/file6/file6.txt.sym.4"
  "${CMAKE_BINARY_DIR}/file8/file8.txt.sym"
  "${CMAKE_BINARY_DIR}/file7/../file7.txt"
  "${CMAKE_BINARY_DIR}/file8.txt"
  "${CMAKE_BINARY_DIR}/file9.sym/file9.txt"
  "${CMAKE_BINARY_DIR}/file10/file10/file10.txt.sym"
  DESTINATION "${CMAKE_BINARY_DIR}/dest2"
  FOLLOW_SYMLINK_CHAIN
  )

set(resolved_file1.txt.sym file1.txt)
set(resolved_file10.txt.sym file10.txt)
set(resolved_file2.txt.sym file2.txt)
set(resolved_file3.txt.sym file3.txt)
set(resolved_file4.txt.sym file4.txt)
set(resolved_file6.txt.sym.1 file6.txt)
set(resolved_file6.txt.sym.2 file6.txt.sym.1)
set(resolved_file6.txt.sym.3 file6.txt.sym.2)
set(resolved_file6.txt.sym.4 file6.txt.sym.3)
set(resolved_file8.txt.sym file8.txt)
set(syms)
foreach(f
  file1.txt
  file1.txt.sym
  file10.txt
  file10.txt.sym
  file2.txt
  file2.txt.sym
  file3.txt
  file3.txt.sym
  file4.txt
  file4.txt.sym
  file5.txt
  file6.txt
  file6.txt.sym.1
  file6.txt.sym.2
  file6.txt.sym.3
  file6.txt.sym.4
  file7.txt
  file8.txt
  file8.txt.sym
  file9.txt
  )
  string(REPLACE "." "\\." r "${f}")
  list(APPEND syms "[^;]*/Tests/RunCMake/file/INSTALL-FOLLOW_SYMLINK_CHAIN-build/dest2/${r}")
  set(filename "${CMAKE_BINARY_DIR}/dest2/${f}")
  if(DEFINED resolved_${f})
    file(READ_SYMLINK "${filename}" resolved)
    if(NOT resolved STREQUAL "${resolved_${f}}")
      message(SEND_ERROR "Expected symlink resolution for ${f}: ${resolved_${f}}\nActual resolution: ${resolved}")
    endif()
  elseif(NOT EXISTS "${filename}" OR IS_SYMLINK "${filename}" OR IS_DIRECTORY "${filename}")
    message(SEND_ERROR "${f} should be a regular file")
  endif()
endforeach()

file(GLOB_RECURSE actual_syms LIST_DIRECTORIES true "${CMAKE_BINARY_DIR}/dest2/*")
if(NOT actual_syms MATCHES "^${syms}$")
  message(SEND_ERROR "Expected files:\n\n  ^${syms}$\n\nActual files:\n\n  ${actual_syms}")
endif()

file(INSTALL
  "${CMAKE_BINARY_DIR}/file1.txt.sym"
  "${CMAKE_BINARY_DIR}/file2/file2.txt.sym"
  "${CMAKE_BINARY_DIR}/file3.txt.sym"
  "${CMAKE_BINARY_DIR}/file4/file4.txt.sym"
  "${CMAKE_BINARY_DIR}/file5.txt"
  "${CMAKE_BINARY_DIR}/file6/file6/file6.txt.sym.4"
  "${CMAKE_BINARY_DIR}/file8/file8.txt.sym"
  "${CMAKE_BINARY_DIR}/file7/../file7.txt"
  "${CMAKE_BINARY_DIR}/file8.txt"
  "${CMAKE_BINARY_DIR}/file9.sym/file9.txt"
  "${CMAKE_BINARY_DIR}/file10/file10/file10.txt.sym"
  DESTINATION "${CMAKE_BINARY_DIR}/dest3"
  )

set(resolved_file1.txt.sym [[^file1\.txt$]])
set(resolved_file10.txt.sym [[/Tests/RunCMake/file/INSTALL-FOLLOW_SYMLINK_CHAIN-build/file10/file10/\.\./file10\.txt$]])
set(resolved_file2.txt.sym [[^\.\./file2\.txt$]])
set(resolved_file3.txt.sym [[/Tests/RunCMake/file/INSTALL-FOLLOW_SYMLINK_CHAIN-build/file3\.txt$]])
set(resolved_file4.txt.sym [[/Tests/RunCMake/file/INSTALL-FOLLOW_SYMLINK_CHAIN-build/file4\.txt$]])
set(resolved_file6.txt.sym.4 [[^file6\.txt\.sym\.3$]])
set(resolved_file8.txt.sym [[/Tests/RunCMake/file/INSTALL-FOLLOW_SYMLINK_CHAIN-build/file8/\.\./file8\.txt$]])
set(syms)
foreach(f
  file1.txt.sym
  file10.txt.sym
  file2.txt.sym
  file3.txt.sym
  file4.txt.sym
  file5.txt
  file6.txt.sym.4
  file7.txt
  file8.txt
  file8.txt.sym
  file9.txt
  )
  string(REPLACE "." "\\." r "${f}")
  list(APPEND syms "[^;]*/Tests/RunCMake/file/INSTALL-FOLLOW_SYMLINK_CHAIN-build/dest3/${r}")
  set(filename "${CMAKE_BINARY_DIR}/dest3/${f}")
  if(DEFINED resolved_${f})
    file(READ_SYMLINK "${filename}" resolved)
    if(NOT resolved MATCHES "${resolved_${f}}")
      message(SEND_ERROR "Expected symlink resolution for ${f}: ${resolved_${f}}\nActual resolution: ${resolved}")
    endif()
  elseif(NOT EXISTS "${filename}" OR IS_SYMLINK "${filename}" OR IS_DIRECTORY "${filename}")
    message(SEND_ERROR "${f} should be a regular file")
  endif()
endforeach()

file(GLOB_RECURSE actual_syms LIST_DIRECTORIES true "${CMAKE_BINARY_DIR}/dest3/*")
if(NOT actual_syms MATCHES "^${syms}$")
  message(SEND_ERROR "Expected files:\n\n  ^${syms}$\n\nActual files:\n\n  ${actual_syms}")
endif()
