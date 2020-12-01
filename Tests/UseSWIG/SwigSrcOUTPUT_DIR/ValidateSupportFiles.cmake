
foreach (support_dir IN LISTS SUPPORT_FILES_DIRECTORY)
  file (GLOB_RECURSE files LIST_DIRECTORIES TRUE RELATIVE "${BASE_DIRECTORY}" "${support_dir}/*")
  list (APPEND support_files ${files})
endforeach()

list(SORT support_files)

if (OUTFILE_DIR)
  set (expected_files "BarSupport/Bar.cs;BarSupport/BarPINVOKE.cs;BarSupport/Math.cs;FooSupport/Foo.cs;FooSupport/FooPINVOKE.cs;FooSupport/Math.cs")
else()
  set (expected_files "Bar/Bar.cs;Bar/BarPINVOKE.cs;Bar/Math.cs;Bar/barCSHARP_wrap.cxx;Foo/Foo.cs;Foo/FooPINVOKE.cs;Foo/Math.cs;Foo/fooCSHARP_wrap.cxx")
endif()

if (NOT support_files STREQUAL expected_files)
  message (FATAL_ERROR "Support files not correctly collected.")
endif()
