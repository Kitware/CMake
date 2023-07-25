set(dir "${CMAKE_CURRENT_BINARY_DIR}")

set(properties
  # property                                expected            alias
  # Compilation properties
  "COMPILE_WARNING_AS_ERROR"                "ON"                "<SAME>"
  "INTERPROCEDURAL_OPTIMIZATION"            "OFF"               "<SAME>"
  "NO_SYSTEM_FROM_IMPORTED"                 "ON"                "<SAME>"
  "VISIBILITY_INLINES_HIDDEN"               "ON"                "<SAME>"
  ## Features
  ### PCH
  "DISABLE_PRECOMPILE_HEADERS"              "ON"                "<SAME>"
  "PCH_WARN_INVALID"                        "OFF"               "<SAME>"
  "PCH_INSTANTIATE_TEMPLATES"               "OFF"               "<SAME>"
  ## Platforms
  ### Android
  "ANDROID_API"                             "9"                 "<SAME>"
  "ANDROID_API_MIN"                         "9"                 "<SAME>"
  "ANDROID_ARCH"                            "arm64-v8a"         "<SAME>"
  "ANDROID_ASSETS_DIRECTORIES"              "${dir}"            "<SAME>"
  "ANDROID_JAVA_SOURCE_DIR"                 "${dir}"            "<SAME>"
  "ANDROID_STL_TYPE"                        "system"            "<SAME>"
  ### macOS
  "OSX_ARCHITECTURES"                       "arm64"             "<SAME>"
  ### Windows
  "MSVC_DEBUG_INFORMATION_FORMAT"           "Embedded"          "<SAME>"
  "MSVC_RUNTIME_LIBRARY"                    "MultiThreaded"     "<SAME>"
  "VS_JUST_MY_CODE_DEBUGGING"               "ON"                "<SAME>"
  ### OpenWatcom
  "WATCOM_RUNTIME_LIBRARY"                  "MultiThreaded"     "<SAME>"
  ## Language
  ### CUDA
  "CUDA_SEPARABLE_COMPILATION"              "ON"                "<SAME>"
  "CUDA_ARCHITECTURES"                      "naive"             "<SAME>"
  ### Fortran
  "Fortran_FORMAT"                          "FREE"              "<SAME>"
  "Fortran_MODULE_DIRECTORY"                "${dir}"            "<SAME>"
  "Fortran_COMPILER_LAUNCHER"               "ccache"            "<SAME>"
  "Fortran_PREPROCESS"                      "ON"                "<SAME>"
  "Fortran_VISIBILITY_PRESET"               "hidden"            "<SAME>"
  ### HIP
  "HIP_ARCHITECTURES"                       "gfx801"            "<SAME>"
  ### ISPC
  "ISPC_COMPILER_LAUNCHER"                  "ccache"            "<SAME>"
  "ISPC_HEADER_DIRECTORY"                   "${dir}"            "<SAME>"
  "ISPC_HEADER_SUFFIX"                      "_i.h"              "<SAME>"
  "ISPC_INSTRUCTION_SETS"                   "avx2-i32x4"        "<SAME>"
  ### Swift
  "Swift_LANGUAGE_VERSION"                  "2.3"               "<SAME>"
  "Swift_MODULE_DIRECTORY"                  "${dir}"            "<SAME>"
  ### moc
  "AUTOMOC"                                 "OFF"               "<SAME>"
  "AUTOMOC_COMPILER_PREDEFINES"             "OFF"               "<SAME>"
  "AUTOMOC_MACRO_NAMES"                     "MOC_CLASS"         "<SAME>"
  "AUTOMOC_MOC_OPTIONS"                     "-v"                "<SAME>"
  "AUTOMOC_PATH_PREFIX"                     "moc_"              "<SAME>"
  "AUTOMOC_EXECUTABLE"                      "automoc"           "<SAME>"
  ### uic
  "AUTOUIC"                                 "OFF"               "<SAME>"
  "AUTOUIC_OPTIONS"                         "-v"                "<SAME>"
  "AUTOUIC_SEARCH_PATHS"                    "${dir}"            "<SAME>"
  "AUTOUIC_EXECUTABLE"                      "autouic"           "<SAME>"
  ### rcc
  "AUTORCC"                                 "OFF"               "<SAME>"
  "AUTORCC_OPTIONS"                         "-v"                "<SAME>"
  "AUTORCC_EXECUTABLE"                      "autorcc"           "<SAME>"

  # Linking properties
  "LINK_SEARCH_START_STATIC"                "-Bstatic"          "<SAME>"
  "LINK_SEARCH_END_STATIC"                  "-Bdynamic"         "<SAME>"
  ## Dependent library lookup
  "MACOSX_RPATH"                            "@loader_path/"     "<SAME>"
  ### Build
  "BUILD_RPATH"                             "../lib"            "<SAME>"
  "BUILD_RPATH_USE_ORIGIN"                  "ON"                "<SAME>"
  "SKIP_BUILD_RPATH"                        "ON"                "<SAME>"
  "BUILD_WITH_INSTALL_RPATH"                "ON"                "<SAME>"
  "BUILD_WITH_INSTALL_NAME_DIR"             "@rpath/"           "<SAME>"
  ### Install
  "INSTALL_NAME_DIR"                        "@rpath/"           "<SAME>"
  "INSTALL_REMOVE_ENVIRONMENT_RPATH"        "ON"                "<SAME>"
  "INSTALL_RPATH"                           "@rpath/"           "<SAME>"
  "INSTALL_RPATH_USE_LINK_PATH"             "ON"                "<SAME>"
  ## Platforms
  ### Android
  "ANDROID_JAR_DIRECTORIES"                 "${dir}"            "<SAME>"
  "ANDROID_JAR_DEPENDENCIES"                "${dir}/foo.jar"    "<SAME>"
  "ANDROID_NATIVE_LIB_DIRECTORIES"          "${dir}"            "<SAME>"
  "ANDROID_NATIVE_LIB_DEPENDENCIES"         "${dir}/native.a"   "<SAME>"
  "ANDROID_PROGUARD"                        "ON"                "<SAME>"
  "ANDROID_PROGUARD_CONFIG_PATH"            "proguard.props"    "<SAME>"
  "ANDROID_SECURE_PROPS_PATH"               "secure.props"      "<SAME>"
  ### iOS
  "IOS_INSTALL_COMBINED"                    "ON"                "<SAME>"
  ### Windows
  "GNUtoMS"                                 "ON"                "<SAME>"
  "WIN32_EXECUTABLE"                        "OFF"               "<SAME>"
  ## Languages
  ### C
  "C_LINKER_LAUNCHER"                       "ccache"            "<SAME>"
  ### C++
  "CXX_LINKER_LAUNCHER"                     "ccache"            "<SAME>"
  ### CUDA
  "CUDA_RESOLVE_DEVICE_SYMBOLS"             "ON"                "<SAME>"
  "CUDA_RUNTIME_LIBRARY"                    "Static"            "<SAME>"
  ### HIP
  "HIP_RUNTIME_LIBRARY"                     "SHARED"            "<SAME>"
  ### Objective C
  "OBJC_LINKER_LAUNCHER"                    "ccache"            "<SAME>"
  ### Objective C++
  "OBJCXX_LINKER_LAUNCHER"                  "ccache"            "<SAME>"

  # Static analysis
  ## C
  "C_CLANG_TIDY"                            "clang-tidy"        "<SAME>"
  "C_CLANG_TIDY_EXPORT_FIXES_DIR"           "${dir}"            "<SAME>"
  "C_CPPLINT"                               "cpplint"           "<SAME>"
  "C_CPPCHECK"                              "cppcheck"          "<SAME>"
  "C_INCLUDE_WHAT_YOU_USE"                  "iwyu"              "<SAME>"
  ## C++
  "CXX_CLANG_TIDY"                          "clang-tidy"        "<SAME>"
  "CXX_CLANG_TIDY_EXPORT_FIXES_DIR"         "${dir}"            "<SAME>"
  "CXX_CPPLINT"                             "cpplint"           "<SAME>"
  "CXX_CPPCHECK"                            "cppcheck"          "<SAME>"
  "CXX_INCLUDE_WHAT_YOU_USE"                "iwyu"              "<SAME>"
  ## Objective C
  "OBJC_CLANG_TIDY"                         "clang-tidy"        "<SAME>"
  "OBJC_CLANG_TIDY_EXPORT_FIXES_DIR"        "${dir}"            "<SAME>"
  ## Objective C++
  "OBJCXX_CLANG_TIDY"                       "clang-tidy"        "<SAME>"
  "OBJCXX_CLANG_TIDY_EXPORT_FIXES_DIR"      "${dir}"            "<SAME>"
  ## Linking
  "LINK_WHAT_YOU_USE"                       "lwyu"              "<SAME>"

  # Build graph properties
  "LINK_DEPENDS_NO_SHARED"                  "OFF"               "<SAME>"
  "UNITY_BUILD"                             "OFF"               "<SAME>"
  "UNITY_BUILD_UNIQUE_ID"                   "unity"             "<SAME>"
  "UNITY_BUILD_BATCH_SIZE"                  "10"                "<SAME>"
  "UNITY_BUILD_MODE"                        "GROUP"             "<SAME>"
  "OPTIMIZE_DEPENDENCIES"                   "ON"                "<SAME>"
  ## Android
  "ANDROID_ANT_ADDITIONAL_OPTIONS"          "-v"                "<SAME>"
  "ANDROID_PROCESS_MAX"                     "2"                 "<SAME>"
  "ANDROID_SKIP_ANT_STEP"                   "ON"                "<SAME>"
  ## Autogen
  "AUTOGEN_ORIGIN_DEPENDS"                  "OFF"               "<SAME>"
  "AUTOGEN_PARALLEL"                        "ON"                "<SAME>"
  "AUTOGEN_USE_SYSTEM_INCLUDE"              "ON"                "<SAME>"
  ## moc
  "AUTOMOC_DEPEND_FILTERS"                  "FIRST<SEMI>SECOND" "<SAME>"
  ## C++
  "CXX_SCAN_FOR_MODULES"                    "ON"                "<SAME>"
  ## Ninja
  "JOB_POOL_COMPILE"                        "compile_pool"      "<SAME>"
  "JOB_POOL_LINK"                           "link_pool"         "<SAME>"
  "JOB_POOL_PRECOMPILE_HEADER"              "pch_pool"          "<SAME>"
  ## Visual Studio
  "VS_NO_COMPILE_BATCHING"                  "ON"                "<SAME>"
  "VS_WINDOWS_TARGET_PLATFORM_MIN_VERSION"  "10.0.10240.0"      "<SAME>"

  # Output location properties
  "ARCHIVE_OUTPUT_DIRECTORY"                "${dir}"            "<SAME>"
  "COMPILE_PDB_OUTPUT_DIRECTORY"            "${dir}"            "<SAME>"
  "LIBRARY_OUTPUT_DIRECTORY"                "${dir}"            "<SAME>"
  "PDB_OUTPUT_DIRECTORY"                    "${dir}"            "<SAME>"
  "RUNTIME_OUTPUT_DIRECTORY"                "${dir}"            "<SAME>"

  # macOS bundle properties
  "FRAMEWORK"                               "OFF"               "<SAME>"
  "FRAMEWORK_MULTI_CONFIG_POSTFIX"          ".mcpostfix"        "<SAME>"
  "MACOSX_BUNDLE"                           "OFF"               "<SAME>"

  # Usage requirement properties
  "LINK_INTERFACE_LIBRARIES"                "c"                 "<SAME>"

  # Metadata
  "EXPORT_COMPILE_COMMANDS"                 "OFF"               "<SAME>"
  )

if (CMAKE_HOST_APPLE) # compile-guarded in CMake
  if (CMAKE_GENERATOR STREQUAL "Xcode")
    list(APPEND properties
      # property                                        expected      alias
      # Xcode properties
      "XCODE_SCHEME_ADDRESS_SANITIZER"                  "ON"          "<SAME>"
      "XCODE_SCHEME_ADDRESS_SANITIZER_USE_AFTER_RETURN" "ON"          "<SAME>"
      "XCODE_SCHEME_DEBUG_DOCUMENT_VERSIONING"          "ON"          "<SAME>"
      "XCODE_SCHEME_ENABLE_GPU_FRAME_CAPTURE_MODE"      "ON"          "<SAME>"
      "XCODE_SCHEME_THREAD_SANITIZER"                   "ON"          "<SAME>"
      "XCODE_SCHEME_THREAD_SANITIZER_STOP"              "ON"          "<SAME>"
      "XCODE_SCHEME_UNDEFINED_BEHAVIOUR_SANITIZER"      "ON"          "<SAME>"
      "XCODE_SCHEME_UNDEFINED_BEHAVIOUR_SANITIZER_STOP" "ON"          "<SAME>"
      "XCODE_SCHEME_LAUNCH_CONFIGURATION"               "ON"          "<SAME>"
      "XCODE_SCHEME_ENABLE_GPU_API_VALIDATION"          "ON"          "<SAME>"
      "XCODE_SCHEME_ENABLE_GPU_SHADER_VALIDATION"       "ON"          "<SAME>"
      "XCODE_SCHEME_WORKING_DIRECTORY"                  "ON"          "<SAME>"
      "XCODE_SCHEME_DISABLE_MAIN_THREAD_CHECKER"        "ON"          "<SAME>"
      "XCODE_SCHEME_MAIN_THREAD_CHECKER_STOP"           "ON"          "<SAME>"
      "XCODE_SCHEME_MALLOC_SCRIBBLE"                    "ON"          "<SAME>"
      "XCODE_SCHEME_MALLOC_GUARD_EDGES"                 "ON"          "<SAME>"
      "XCODE_SCHEME_GUARD_MALLOC"                       "ON"          "<SAME>"
      "XCODE_SCHEME_LAUNCH_MODE"                        "ON"          "<SAME>"
      "XCODE_SCHEME_ZOMBIE_OBJECTS"                     "ON"          "<SAME>"
      "XCODE_SCHEME_MALLOC_STACK"                       "ON"          "<SAME>"
      "XCODE_SCHEME_DYNAMIC_LINKER_API_USAGE"           "ON"          "<SAME>"
      "XCODE_SCHEME_DYNAMIC_LIBRARY_LOADS"              "ON"          "<SAME>"
      "XCODE_SCHEME_ENVIRONMENT"                        "ON"          "<SAME>"
      "XCODE_LINK_BUILD_PHASE_MODE"                     "BUILT_ONLY"  "<SAME>"
      )
  endif ()
endif ()

macro (add_language_properties lang std)
  list(APPEND properties
    # property                      expected  alias
    "${lang}_COMPILER_LAUNCHER"     "ccache"  "<SAME>"
    "${lang}_STANDARD"              "${std}"  "<SAME>"
    "${lang}_STANDARD_REQUIRED"     "TRUE"    "<SAME>"
    "${lang}_EXTENSIONS"            "FALSE"   "<SAME>"
    "${lang}_VISIBILITY_PRESET"     "hidden"  "<SAME>"
    )
endmacro ()

# Mock up knowing the standard flag. This doesn't actually build, so nothing
# should care at this point.
set(CMAKE_Cc_std_11_STANDARD_COMPILE_OPTION "-std=c11")

add_language_properties(C c_std_11)
add_language_properties(CXX cxx_std_11)
add_language_properties(CUDA cuda_std_11)
add_language_properties(HIP hip_std_11)
add_language_properties(OBJC c_std_99)
add_language_properties(OBJCXX cxx_std_11)

# Set up pools for properties set above.
if (CMAKE_GENERATOR MATCHES "Ninja")
  set_property(GLOBAL APPEND
    PROPERTY
      JOB_POOLS
        compile_pool=1
        link_pool=1
        pch_pool=1)
endif ()

prepare_target_types(can_compile_sources
  EXECUTABLE SHARED STATIC MODULE OBJECT)

run_property_tests(can_compile_sources properties)

set(properties_with_defaults
  # property                      expected  alias
  "PCH_WARN_INVALID"              "ON"      "<SAME>"
  "PCH_INSTANTIATE_TEMPLATES"     "ON"      "<SAME>"
  "ISPC_HEADER_SUFFIX"            "_ispc.h" "<SAME>"
  "SKIP_BUILD_RPATH"              "OFF"     "<SAME>"
  "BUILD_WITH_INSTALL_RPATH"      "OFF"     "<SAME>"
  "INSTALL_RPATH"                 ""        "<SAME>"
  "INSTALL_RPATH_USE_LINK_PATH"   "OFF"     "<SAME>"
  "UNITY_BUILD_BATCH_SIZE"        "8"       "<SAME>"
  "UNITY_BUILD_MODE"              "BATCH"   "<SAME>"
  )

if (CMAKE_HOST_APPLE)
  if (CMAKE_GENERATOR STREQUAL "Xcode")
    list(APPEND properties_with_defaults
      # property                      expected  alias
      "XCODE_LINK_BUILD_PHASE_MODE"   "NONE"    "<SAME>"
      )
  endif ()
endif ()

set(with_defaults 1)
run_property_tests(can_compile_sources properties_with_defaults)
