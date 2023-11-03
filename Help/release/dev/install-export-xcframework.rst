install-export-xcframework
--------------------------

* The :command:`export(SETUP)` command gained a new ``XCFRAMEWORK_LOCATION``
  argument, which can be used to specify the location of a ``.xcframework``
  that can be substituted for the installed library.
* The :module:`CMakePackageConfigHelpers` module gained a new
  :command:`generate_apple_platform_selection_file` function, which can be
  used to generate a file that includes another Apple-platform-specific file.
