include(Compiler/XL)
__compiler_xl(CXX)
set(CMAKE_CXX_FLAGS_RELEASE_INIT "${CMAKE_CXX_FLAGS_RELEASE_INIT} -DNDEBUG")
set(CMAKE_CXX_FLAGS_MINSIZEREL_INIT "${CMAKE_CXX_FLAGS_MINSIZEREL_INIT} -DNDEBUG")

# -qthreaded     = Ensures that all optimizations will be thread-safe
# -qhalt=e       = Halt on error messages (rather than just severe errors)
set(CMAKE_CXX_FLAGS_INIT "-qthreaded -qhalt=e")

set(CMAKE_CXX_COMPILE_OBJECT
  "<CMAKE_CXX_COMPILER> -+ <DEFINES> <FLAGS> -o <OBJECT> -c <SOURCE>")
set(_CMAKE_CXX_CREATE_OBJECT_FILE "${CMAKE_CXX_COMPILER};<FLAGS>;-c;<SOURCE>")

set(CMAKE_CXX11_COMPILER_FEATURES)

include("${CMAKE_ROOT}/Modules/Internal/FeatureTesting.cmake")

if (NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS 10.1)
   # TODO: See comment 4 here: https://www.ibm.com/developerworks/community/blogs/5894415f-be62-4bc0-81c5-3956e82276f3/entry/xlc_compiler_s_c_11_support50?lang=en
   # http://pic.dhe.ibm.com/infocenter/lnxpcomp/v121v141/index.jsp says c++0x support is experimental and should not be relied upon. Maybe we should not enable it
   # for this compiler.
   # http://pic.dhe.ibm.com/infocenter/lnxpcomp/v121v141/index.jsp?topic=%2Fcom.ibm.xlcpp121.linux.doc%2Flanguage_ref%2Fcpp0x_exts.html says that IBM extensions
   # are enabled by default. We disable that stuff for GNU unless a non-portable feature is used. Do the same here?
   set(CMAKE_CXX11_STANDARD_COMPILE_OPTION "-qlanglvl=extended0x")
   record_compiler_features(CXX "-qlanglvl=extended0x" CMAKE_CXX11_COMPILER_FEATURES)
endif()


set(CMAKE_CXX_COMPILER_FEATURES
  ${CMAKE_CXX11_COMPILER_FEATURES}
)
