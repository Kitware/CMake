Note to authors of FindXXX.cmake files

We would like all FindXXX.cmake files to produce consistent variable names.

XXX_INCLUDE_DIR, 	Where to find xxx.h, etc.
XXX_LIBRARIES, 		The libraries to link against to use XXX. These should include full paths.
XXX_DEFINITIONS, 	Definitions to use when compiling code that uses XXX.
XXX_EXECUTABLE, 	Where to find the XXX tool.
XXX_YYY_EXECUTABLE, 	Where to find the YYY tool that comes with XXX.
XXX_ROOT_DIR, 		Where to find the home directory of XXX.
XXX_FOUND, 		Set to false if we haven't found, or don't want to use XXX.


You do not have to provide all of the above variables. You should provide XXX_FOUND under most circumstances. If XXX is a library, then XXX_INCLUDE_DIR, XXX_LIBRARIES, and XXX_DEFINITIONS should also be defined.

Try to keep as many options as possible out of the cache, leaving at least one option which can be used to disable use of the module, or find a lost library (e.g. XXX_ROOT_DIR)

If you need other commands to do special things (e.g. the QT_WRAP_UI setting in FindQt.cmake) then it should still begin with XXX_. This gives a sort of namespace effect.
