set_source_files_properties(a.txt DIRECTORY PROPERTIES COMPILE_DEFINITIONS "def")
set_source_files_properties(a.txt DIRECTORY non_existing_dir PROPERTIES COMPILE_DEFINITIONS "def")
set_source_files_properties(a.txt TARGET_DIRECTORY PROPERTIES COMPILE_DEFINITIONS "def")
set_source_files_properties(a.txt TARGET_DIRECTORY non_existing_target PROPERTIES COMPILE_DEFINITIONS "def")

get_property(in_var SOURCE a.txt DIRECTORY PROPERTY COMPILE_DEFINITIONS)
get_property(in_var SOURCE a.txt DIRECTORY non_existing_dir PROPERTY COMPILE_DEFINITIONS)
get_property(in_var SOURCE a.txt TARGET_DIRECTORY PROPERTY COMPILE_DEFINITIONS)
get_property(in_var SOURCE a.txt TARGET_DIRECTORY non_existing_dir PROPERTY COMPILE_DEFINITIONS)

get_source_file_property(in_var a.txt DIRECTORY PROPERTY COMPILE_DEFINITIONS)
get_source_file_property(in_var a.txt DIRECTORY non_existing_dir PROPERTY COMPILE_DEFINITIONS)
get_source_file_property(in_var a.txt TARGET_DIRECTORY PROPERTY COMPILE_DEFINITIONS)
get_source_file_property(in_var a.txt TARGET_DIRECTORY non_existing_dir PROPERTY COMPILE_DEFINITIONS)
