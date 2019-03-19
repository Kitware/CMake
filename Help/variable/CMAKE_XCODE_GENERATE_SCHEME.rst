CMAKE_XCODE_GENERATE_SCHEME
---------------------------

If enabled, the Xcode generator will generate schema files.  These
are useful to invoke analyze, archive, build-for-testing and test
actions from the command line.

The following target properties overwrite the default of the
corresponding settings on the "Diagnostic" tab for each schema file.
Each of those is initialized by the respective ``CMAKE_`` variable
at target creation time.

- :prop_tgt:`XCODE_SCHEME_ADDRESS_SANITIZER`
- :prop_tgt:`XCODE_SCHEME_ADDRESS_SANITIZER_USE_AFTER_RETURN`
- :prop_tgt:`XCODE_SCHEME_DISABLE_MAIN_THREAD_CHECKER`
- :prop_tgt:`XCODE_SCHEME_DYNAMIC_LIBRARY_LOADS`
- :prop_tgt:`XCODE_SCHEME_DYNAMIC_LINKER_API_USAGE`
- :prop_tgt:`XCODE_SCHEME_GUARD_MALLOC`
- :prop_tgt:`XCODE_SCHEME_MAIN_THREAD_CHECKER_STOP`
- :prop_tgt:`XCODE_SCHEME_MALLOC_GUARD_EDGES`
- :prop_tgt:`XCODE_SCHEME_MALLOC_SCRIBBLE`
- :prop_tgt:`XCODE_SCHEME_MALLOC_STACK`
- :prop_tgt:`XCODE_SCHEME_THREAD_SANITIZER`
- :prop_tgt:`XCODE_SCHEME_THREAD_SANITIZER_STOP`
- :prop_tgt:`XCODE_SCHEME_UNDEFINED_BEHAVIOUR_SANITIZER`
- :prop_tgt:`XCODE_SCHEME_UNDEFINED_BEHAVIOUR_SANITIZER_STOP`
- :prop_tgt:`XCODE_SCHEME_ZOMBIE_OBJECTS`

The following target properties will be applied on the
"Info" and "Arguments" tab:

- :prop_tgt:`XCODE_SCHEME_ARGUMENTS`
- :prop_tgt:`XCODE_SCHEME_DEBUG_AS_ROOT`
- :prop_tgt:`XCODE_SCHEME_ENVIRONMENT`
- :prop_tgt:`XCODE_SCHEME_EXECUTABLE`
