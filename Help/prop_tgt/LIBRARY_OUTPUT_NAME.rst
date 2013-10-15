LIBRARY_OUTPUT_NAME
-------------------

Output name for LIBRARY target files.

This property specifies the base name for library target files.  It
overrides OUTPUT_NAME and OUTPUT_NAME_<CONFIG> properties.  There are
three kinds of target files that may be built: archive, library, and
runtime.  Executables are always treated as runtime targets.  Static
libraries are always treated as archive targets.  Module libraries are
always treated as library targets.  For non-DLL platforms shared
libraries are treated as library targets.  For DLL platforms the DLL
part of a shared library is treated as a runtime target and the
corresponding import library is treated as an archive target.  All
Windows-based systems including Cygwin are DLL platforms.
