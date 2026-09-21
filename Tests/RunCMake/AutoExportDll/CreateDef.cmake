# Run "cmake -E __create_def" on the checked-in object files and compare the
# generated .def files with the expected ones.  The object files are compiled
# for Windows but are read on every host, so this checks that the COFF symbol
# table backend used on Windows hosts and the nm backend used on other
# hosts export the same symbols.
#
# The object files were generated from CreateDef/fixture.cxx with
#
#   clang-cl --target=<arch>-pc-windows-msvc /c /GR- /EHs- /Os /Zl -nostdinc \
#     CreateDef/fixture.cxx /Fo:CreateDef/fixture-<arch>.obj
#
# once per architecture.

set(failed "")
foreach(arch IN ITEMS x86_64 i386 arm64ec)
  set(objlist "${bin}/${arch}-objects.txt")
  set(def "${bin}/${arch}.def")
  file(WRITE "${objlist}" "${src}/CreateDef/fixture-${arch}.obj\n")
  file(REMOVE "${def}")
  execute_process(
    COMMAND ${cmake} -E __create_def "${def}" "${objlist}" ${nm}
    RESULT_VARIABLE result
    OUTPUT_VARIABLE output
    ERROR_VARIABLE output
    )
  if(NOT result EQUAL 0)
    string(APPEND failed
      "__create_def failed for ${arch}: ${result}\n${output}")
    continue()
  endif()
  file(READ "${def}" actual)
  file(READ "${src}/CreateDef/fixture-${arch}.def" expect)
  # Compare ignoring trailing whitespace, which __create_def emits.
  string(REGEX REPLACE "[ \t\r]+\n" "\n" actual "${actual}")
  string(REGEX REPLACE "[ \t\r]+\n" "\n" expect "${expect}")
  if(NOT actual STREQUAL expect)
    string(APPEND failed
      "Wrong exports for ${arch}.\n"
      "Expected:\n${expect}\n"
      "Actual:\n${actual}\n")
  endif()
endforeach()

if(failed)
  message(FATAL_ERROR "${failed}")
endif()
