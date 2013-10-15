LIBRARY_OUTPUT_DIRECTORY
------------------------

Output directory in which to build LIBRARY target files.

This property specifies the directory into which library target files
should be built.  Multi-configuration generators (VS, Xcode) append a
per-configuration subdirectory to the specified directory.  There are
three kinds of target files that may be built: archive, library, and
runtime.  Executables are always treated as runtime targets.  Static
libraries are always treated as archive targets.  Module libraries are
always treated as library targets.  For non-DLL platforms shared
libraries are treated as library targets.  For DLL platforms the DLL
part of a shared library is treated as a runtime target and the
corresponding import library is treated as an archive target.  All
Windows-based systems including Cygwin are DLL platforms.  This
property is initialized by the value of the variable
CMAKE_LIBRARY_OUTPUT_DIRECTORY if it is set when a target is created.
