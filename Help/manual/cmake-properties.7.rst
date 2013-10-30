.. cmake-manual-description: CMake Properties Reference

cmake-properties(7)
*******************

.. only:: html or latex

   .. contents::

Properties of Global Scope
==========================

.. toctree::
   /prop_gbl/ALLOW_DUPLICATE_CUSTOM_TARGETS
   /prop_gbl/AUTOGEN_TARGETS_FOLDER
   /prop_gbl/AUTOMOC_TARGETS_FOLDER
   /prop_gbl/DEBUG_CONFIGURATIONS
   /prop_gbl/DISABLED_FEATURES
   /prop_gbl/ENABLED_FEATURES
   /prop_gbl/ENABLED_LANGUAGES
   /prop_gbl/FIND_LIBRARY_USE_LIB64_PATHS
   /prop_gbl/FIND_LIBRARY_USE_OPENBSD_VERSIONING
   /prop_gbl/GLOBAL_DEPENDS_DEBUG_MODE
   /prop_gbl/GLOBAL_DEPENDS_NO_CYCLES
   /prop_gbl/IN_TRY_COMPILE
   /prop_gbl/PACKAGES_FOUND
   /prop_gbl/PACKAGES_NOT_FOUND
   /prop_gbl/PREDEFINED_TARGETS_FOLDER
   /prop_gbl/REPORT_UNDEFINED_PROPERTIES
   /prop_gbl/RULE_LAUNCH_COMPILE
   /prop_gbl/RULE_LAUNCH_CUSTOM
   /prop_gbl/RULE_LAUNCH_LINK
   /prop_gbl/RULE_MESSAGES
   /prop_gbl/TARGET_ARCHIVES_MAY_BE_SHARED_LIBS
   /prop_gbl/TARGET_SUPPORTS_SHARED_LIBS
   /prop_gbl/USE_FOLDERS

Properties on Directories
=========================

.. toctree::
   /prop_dir/ADDITIONAL_MAKE_CLEAN_FILES
   /prop_dir/CACHE_VARIABLES
   /prop_dir/CLEAN_NO_CUSTOM
   /prop_dir/COMPILE_DEFINITIONS_CONFIG
   /prop_dir/COMPILE_DEFINITIONS
   /prop_dir/COMPILE_OPTIONS
   /prop_dir/DEFINITIONS
   /prop_dir/EXCLUDE_FROM_ALL
   /prop_dir/IMPLICIT_DEPENDS_INCLUDE_TRANSFORM
   /prop_dir/INCLUDE_DIRECTORIES
   /prop_dir/INCLUDE_REGULAR_EXPRESSION
   /prop_dir/INTERPROCEDURAL_OPTIMIZATION_CONFIG
   /prop_dir/INTERPROCEDURAL_OPTIMIZATION
   /prop_dir/LINK_DIRECTORIES
   /prop_dir/LISTFILE_STACK
   /prop_dir/MACROS
   /prop_dir/PARENT_DIRECTORY
   /prop_dir/RULE_LAUNCH_COMPILE
   /prop_dir/RULE_LAUNCH_CUSTOM
   /prop_dir/RULE_LAUNCH_LINK
   /prop_dir/TEST_INCLUDE_FILE
   /prop_dir/VARIABLES
   /prop_dir/VS_GLOBAL_SECTION_POST_section
   /prop_dir/VS_GLOBAL_SECTION_PRE_section

Properties on Targets
=====================

.. toctree::
   /prop_tgt/ALIASED_TARGET
   /prop_tgt/ARCHIVE_OUTPUT_DIRECTORY_CONFIG
   /prop_tgt/ARCHIVE_OUTPUT_DIRECTORY
   /prop_tgt/ARCHIVE_OUTPUT_NAME_CONFIG
   /prop_tgt/ARCHIVE_OUTPUT_NAME
   /prop_tgt/AUTOMOC_MOC_OPTIONS
   /prop_tgt/AUTOMOC
   /prop_tgt/AUTOUIC
   /prop_tgt/AUTOUIC_OPTIONS
   /prop_tgt/AUTORCC
   /prop_tgt/AUTORCC_OPTIONS
   /prop_tgt/BUILD_WITH_INSTALL_RPATH
   /prop_tgt/BUNDLE_EXTENSION
   /prop_tgt/BUNDLE
   /prop_tgt/COMPATIBLE_INTERFACE_BOOL
   /prop_tgt/COMPATIBLE_INTERFACE_NUMBER_MAX
   /prop_tgt/COMPATIBLE_INTERFACE_NUMBER_MIN
   /prop_tgt/COMPATIBLE_INTERFACE_STRING
   /prop_tgt/COMPILE_DEFINITIONS_CONFIG
   /prop_tgt/COMPILE_DEFINITIONS
   /prop_tgt/COMPILE_FLAGS
   /prop_tgt/COMPILE_OPTIONS
   /prop_tgt/CONFIG_OUTPUT_NAME
   /prop_tgt/CONFIG_POSTFIX
   /prop_tgt/DEBUG_POSTFIX
   /prop_tgt/DEFINE_SYMBOL
   /prop_tgt/EchoString
   /prop_tgt/ENABLE_EXPORTS
   /prop_tgt/EXCLUDE_FROM_ALL
   /prop_tgt/EXCLUDE_FROM_DEFAULT_BUILD_CONFIG
   /prop_tgt/EXCLUDE_FROM_DEFAULT_BUILD
   /prop_tgt/EXPORT_NAME
   /prop_tgt/FOLDER
   /prop_tgt/Fortran_FORMAT
   /prop_tgt/Fortran_MODULE_DIRECTORY
   /prop_tgt/FRAMEWORK
   /prop_tgt/GENERATOR_FILE_NAME
   /prop_tgt/GNUtoMS
   /prop_tgt/HAS_CXX
   /prop_tgt/IMPLICIT_DEPENDS_INCLUDE_TRANSFORM
   /prop_tgt/IMPORTED_CONFIGURATIONS
   /prop_tgt/IMPORTED_IMPLIB_CONFIG
   /prop_tgt/IMPORTED_IMPLIB
   /prop_tgt/IMPORTED_LINK_DEPENDENT_LIBRARIES_CONFIG
   /prop_tgt/IMPORTED_LINK_DEPENDENT_LIBRARIES
   /prop_tgt/IMPORTED_LINK_INTERFACE_LANGUAGES_CONFIG
   /prop_tgt/IMPORTED_LINK_INTERFACE_LANGUAGES
   /prop_tgt/IMPORTED_LINK_INTERFACE_LIBRARIES_CONFIG
   /prop_tgt/IMPORTED_LINK_INTERFACE_LIBRARIES
   /prop_tgt/IMPORTED_LINK_INTERFACE_MULTIPLICITY_CONFIG
   /prop_tgt/IMPORTED_LINK_INTERFACE_MULTIPLICITY
   /prop_tgt/IMPORTED_LOCATION_CONFIG
   /prop_tgt/IMPORTED_LOCATION
   /prop_tgt/IMPORTED_NO_SONAME_CONFIG
   /prop_tgt/IMPORTED_NO_SONAME
   /prop_tgt/IMPORTED
   /prop_tgt/IMPORTED_SONAME_CONFIG
   /prop_tgt/IMPORTED_SONAME
   /prop_tgt/IMPORT_PREFIX
   /prop_tgt/IMPORT_SUFFIX
   /prop_tgt/INCLUDE_DIRECTORIES
   /prop_tgt/INSTALL_NAME_DIR
   /prop_tgt/INSTALL_RPATH
   /prop_tgt/INSTALL_RPATH_USE_LINK_PATH
   /prop_tgt/INTERFACE_COMPILE_DEFINITIONS
   /prop_tgt/INTERFACE_COMPILE_OPTIONS
   /prop_tgt/INTERFACE_INCLUDE_DIRECTORIES
   /prop_tgt/INTERFACE_LINK_LIBRARIES
   /prop_tgt/INTERFACE_POSITION_INDEPENDENT_CODE
   /prop_tgt/INTERFACE_SYSTEM_INCLUDE_DIRECTORIES
   /prop_tgt/INTERPROCEDURAL_OPTIMIZATION_CONFIG
   /prop_tgt/INTERPROCEDURAL_OPTIMIZATION
   /prop_tgt/LABELS
   /prop_tgt/LANG_VISIBILITY_PRESET
   /prop_tgt/LIBRARY_OUTPUT_DIRECTORY_CONFIG
   /prop_tgt/LIBRARY_OUTPUT_DIRECTORY
   /prop_tgt/LIBRARY_OUTPUT_NAME_CONFIG
   /prop_tgt/LIBRARY_OUTPUT_NAME
   /prop_tgt/LINK_DEPENDS_NO_SHARED
   /prop_tgt/LINK_DEPENDS
   /prop_tgt/LINKER_LANGUAGE
   /prop_tgt/LINK_FLAGS_CONFIG
   /prop_tgt/LINK_FLAGS
   /prop_tgt/LINK_INTERFACE_LIBRARIES_CONFIG
   /prop_tgt/LINK_INTERFACE_LIBRARIES
   /prop_tgt/LINK_INTERFACE_MULTIPLICITY_CONFIG
   /prop_tgt/LINK_INTERFACE_MULTIPLICITY
   /prop_tgt/LINK_LIBRARIES
   /prop_tgt/LINK_SEARCH_END_STATIC
   /prop_tgt/LINK_SEARCH_START_STATIC
   /prop_tgt/LOCATION_CONFIG
   /prop_tgt/LOCATION
   /prop_tgt/MACOSX_BUNDLE_INFO_PLIST
   /prop_tgt/MACOSX_BUNDLE
   /prop_tgt/MACOSX_FRAMEWORK_INFO_PLIST
   /prop_tgt/MACOSX_RPATH
   /prop_tgt/MAP_IMPORTED_CONFIG_CONFIG
   /prop_tgt/NAME
   /prop_tgt/NO_SONAME
   /prop_tgt/NO_SYSTEM_FROM_IMPORTED
   /prop_tgt/OSX_ARCHITECTURES_CONFIG
   /prop_tgt/OSX_ARCHITECTURES
   /prop_tgt/OUTPUT_NAME_CONFIG
   /prop_tgt/OUTPUT_NAME
   /prop_tgt/PDB_NAME_CONFIG
   /prop_tgt/PDB_NAME
   /prop_tgt/PDB_OUTPUT_DIRECTORY_CONFIG
   /prop_tgt/PDB_OUTPUT_DIRECTORY
   /prop_tgt/POSITION_INDEPENDENT_CODE
   /prop_tgt/POST_INSTALL_SCRIPT
   /prop_tgt/PREFIX
   /prop_tgt/PRE_INSTALL_SCRIPT
   /prop_tgt/PRIVATE_HEADER
   /prop_tgt/PROJECT_LABEL
   /prop_tgt/PUBLIC_HEADER
   /prop_tgt/RESOURCE
   /prop_tgt/RULE_LAUNCH_COMPILE
   /prop_tgt/RULE_LAUNCH_CUSTOM
   /prop_tgt/RULE_LAUNCH_LINK
   /prop_tgt/RUNTIME_OUTPUT_DIRECTORY_CONFIG
   /prop_tgt/RUNTIME_OUTPUT_DIRECTORY
   /prop_tgt/RUNTIME_OUTPUT_NAME_CONFIG
   /prop_tgt/RUNTIME_OUTPUT_NAME
   /prop_tgt/SKIP_BUILD_RPATH
   /prop_tgt/SOURCES
   /prop_tgt/SOVERSION
   /prop_tgt/STATIC_LIBRARY_FLAGS_CONFIG
   /prop_tgt/STATIC_LIBRARY_FLAGS
   /prop_tgt/SUFFIX
   /prop_tgt/TYPE
   /prop_tgt/VERSION
   /prop_tgt/VISIBILITY_INLINES_HIDDEN
   /prop_tgt/VS_DOTNET_REFERENCES
   /prop_tgt/VS_DOTNET_TARGET_FRAMEWORK_VERSION
   /prop_tgt/VS_GLOBAL_KEYWORD
   /prop_tgt/VS_GLOBAL_PROJECT_TYPES
   /prop_tgt/VS_GLOBAL_ROOTNAMESPACE
   /prop_tgt/VS_GLOBAL_variable
   /prop_tgt/VS_KEYWORD
   /prop_tgt/VS_SCC_AUXPATH
   /prop_tgt/VS_SCC_LOCALPATH
   /prop_tgt/VS_SCC_PROJECTNAME
   /prop_tgt/VS_SCC_PROVIDER
   /prop_tgt/VS_WINRT_EXTENSIONS
   /prop_tgt/VS_WINRT_REFERENCES
   /prop_tgt/WIN32_EXECUTABLE
   /prop_tgt/XCODE_ATTRIBUTE_an-attribute

Properties on Tests
===================

.. toctree::
   /prop_test/ATTACHED_FILES_ON_FAIL
   /prop_test/ATTACHED_FILES
   /prop_test/COST
   /prop_test/DEPENDS
   /prop_test/ENVIRONMENT
   /prop_test/FAIL_REGULAR_EXPRESSION
   /prop_test/LABELS
   /prop_test/MEASUREMENT
   /prop_test/PASS_REGULAR_EXPRESSION
   /prop_test/PROCESSORS
   /prop_test/REQUIRED_FILES
   /prop_test/RESOURCE_LOCK
   /prop_test/RUN_SERIAL
   /prop_test/TIMEOUT
   /prop_test/WILL_FAIL
   /prop_test/WORKING_DIRECTORY

Properties on Source Files
==========================

.. toctree::
   /prop_sf/ABSTRACT
   /prop_sf/AUTOUIC_OPTIONS
   /prop_sf/AUTORCC_OPTIONS
   /prop_sf/COMPILE_DEFINITIONS_CONFIG
   /prop_sf/COMPILE_DEFINITIONS
   /prop_sf/COMPILE_FLAGS
   /prop_sf/EXTERNAL_OBJECT
   /prop_sf/Fortran_FORMAT
   /prop_sf/GENERATED
   /prop_sf/HEADER_FILE_ONLY
   /prop_sf/KEEP_EXTENSION
   /prop_sf/LABELS
   /prop_sf/LANGUAGE
   /prop_sf/LOCATION
   /prop_sf/MACOSX_PACKAGE_LOCATION
   /prop_sf/OBJECT_DEPENDS
   /prop_sf/OBJECT_OUTPUTS
   /prop_sf/SYMBOLIC
   /prop_sf/WRAP_EXCLUDE

Properties on Cache Entries
===========================

.. toctree::
   /prop_cache/ADVANCED
   /prop_cache/HELPSTRING
   /prop_cache/MODIFIED
   /prop_cache/STRINGS
   /prop_cache/TYPE
   /prop_cache/VALUE
