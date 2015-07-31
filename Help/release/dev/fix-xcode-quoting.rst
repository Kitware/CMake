fix-xcode-quoting
-----------------

* During CMake 3.3 development a patch landed in CMake which made the
  CMake Xcode generator produce project files just like Xcode does.
  Xcode does not quote strings containing a period. That's why CMake
  stopped quoting those, too. But Xcode needs quoting for string with
  a tilde like ``icon29x29~ipad.png`` which were covered before due to
  the containing period. CMake now properly quotes strings containing
  a tilde.