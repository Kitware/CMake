Note to authors of FindXXX.cmake files

We would like all FindXXX.cmake files to produce consistent variable names.

Please use the following consistent variable names for general use.

XXX_INCLUDE_DIR        	Where to find xxx.h, etc. If for some reason, you really need two paths, then that shouldn't be a problem - however, consider if you really should have two different FindXXX.cmake files. (XXX_INCLUDE_PATH was considered bad because a path includes an actual filename.)
XXX_LIBRARIES          	The libraries to link against to use XXX. These should include full paths.
XXX_DEFINITIONS        	Definitions to use when compiling code that uses XXX. This really shouldn't include options such as (-DHAS_JPEG)that a client source-code file uses to decide whether to #include <jpeg.h>
XXX_EXECUTABLE         	Where to find the XXX tool.
XXX_YYY_EXECUTABLE     	Where to find the YYY tool that comes with XXX.
XXX_ROOT_DIR           	Where to find the base directory of XXX.
XXX_VERSION_YY		Expect Version YY if true. Make sure at most one of these is ever true.
XXX_WRAP_YY		If False, do not try to use the relevent CMake wrapping command.
XXX_YY_FOUND           	If False, optional YY part of XXX sytem is not available.
XXX_FOUND              	Set to false, or undefined, if we haven't found, or don't want to use XXX.

You do not have to provide all of the above variables. You should provide XXX_FOUND under most circumstances. If XXX is a library, then  XXX_LIBRARIES, should also be defined, and XXX_INCLUDE_DIR should usually be defined (I guess libm.a might be an exception)

The following names should usually be used in CMakeLists.txt files, but they may be usefully modified in users CMake Cache to control stuff.

XXX_LIBRARY		Name of XXX Library. A User may set this and XXX_INCLUDE_DIR to ignore to force non-use of XXX.
XXX_YY_LIBRARY		Name of YY library that is part of the XXX system. It may or may not be required to use XXX.

For tidiness's sake, try to keep as many options as possible out of the cache, leaving at least one option which can be used to disable use of the module, or locate a not-found library (e.g. XXX_ROOT_DIR). For the same reason, mark most cache options as advanced.

If you need other commands to do special things then it should still begin with XXX_. This gives a sort of namespace effect and keeps things tidy for the user. You should put comments describing all the exported settings, plus descriptions of any the users can use to control stuff.

You really should also provide backwards compatibility any old settings that were actually in use. Make sure you comment them as deprecated, so that no-one starts using them.