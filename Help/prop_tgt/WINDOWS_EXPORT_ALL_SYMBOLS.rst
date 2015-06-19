WINDOWS_EXPORT_ALL_SYMBOLS
--------------------------

This property is implemented only when the compiler supports it.

This property will automatically create a .def file with all global
symbols found in the input .obj files for a dll on Windows. The def
file will be passed to the linker causing all symbols to be exported
from the dll. For any global data __declspec(dllimport) must still be
used when compiling against the code in the dll. All other function
symbols will be automatically exported and imported by callers.

This property is initialized by the value of
the :variable:`CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS` variable if it is set
when a target is created.
