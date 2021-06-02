include("${CMAKE_CURRENT_LIST_DIR}/../check_installed.cmake")

if(CMAKE_HOST_SYSTEM_NAME STREQUAL "Linux")
  set(_dirs [[aaa;aaa/executables;aaa/executables/testExe1-4;aaa/executables/testExe3;aaa/libraries;aaa/libraries/libtestExe2lib\.so;aaa/libraries/libtestLib3lib(-d|-r)?\.so\.1\.2;aaa/libraries/libtestLib3lib(-d|-r)?\.so\.3;aaa/libraries/libtestLib4\.so;aaa/libraries/libtestMod1\.so;aaa/libraries/libtestMod2\.so]])
  set(_alldest [[bbb;bbb/libtestExe2lib\.so;bbb/libtestLib3lib(-d|-r)?\.so\.1\.2;bbb/libtestLib3lib(-d|-r)?\.so\.3;bbb/libtestLib4\.so;bbb/libtestMod1\.so;bbb/libtestMod2\.so;bbb/testExe1-4;bbb/testExe3]])
  set(_default [[bin;bin/testExe1-4;bin/testExe3;lib;lib/libtestExe2lib\.so;lib/libtestLib3lib(-d|-r)?\.so\.1\.2;lib/libtestLib3lib(-d|-r)?\.so\.3;lib/libtestLib4\.so;lib/libtestMod1\.so;lib/libtestMod2\.so]])
  check_installed("^${_dirs};${_alldest};${_default}$")
elseif(CMAKE_HOST_SYSTEM_NAME STREQUAL "Windows")
  set(_dirs_msvc [[aaa;aaa/executables;aaa/executables/testExe1\.exe;aaa/executables/testExe2lib\.dll;aaa/executables/testExe3\.exe;aaa/executables/testLib3dll(-d|-r)?\.dll;aaa/executables/testLib4\.dll;aaa/libraries;aaa/libraries/testMod1\.dll;aaa/libraries/testMod2\.dll]])
  set(_dirs_mingw [[aaa;aaa/executables;aaa/executables/libtestExe2lib\.dll;aaa/executables/libtestLib3dll(-d|-r)?\.dll;aaa/executables/libtestLib4\.dll;aaa/executables/testExe1\.exe;aaa/executables/testExe3\.exe;aaa/libraries;aaa/libraries/libtestMod1\.dll;aaa/libraries/libtestMod2\.dll]])

  set(_alldest_msvc [[bbb;bbb/testExe1\.exe;bbb/testExe2lib\.dll;bbb/testExe3\.exe;bbb/testLib3dll(-d|-r)?\.dll;bbb/testLib4\.dll;bbb/testMod1\.dll;bbb/testMod2\.dll]])
  set(_alldest_mingw [[bbb;bbb/libtestExe2lib\.dll;bbb/libtestLib3dll(-d|-r)?\.dll;bbb/libtestLib4\.dll;bbb/libtestMod1\.dll;bbb/libtestMod2\.dll;bbb/testExe1\.exe;bbb/testExe3\.exe]])

  set(_default_msvc [[bin;bin/testExe1\.exe;bin/testExe2lib\.dll;bin/testExe3\.exe;bin/testLib3dll(-d|-r)?\.dll;bin/testLib4\.dll;lib;lib/testMod1\.dll;lib/testMod2\.dll]])
  set(_default_mingw [[bin;bin/libtestExe2lib\.dll;bin/libtestLib3dll(-d|-r)?\.dll;bin/libtestLib4\.dll;bin/testExe1\.exe;bin/testExe3\.exe;lib;lib/libtestMod1\.dll;lib/libtestMod2\.dll]])

  check_installed("^(${_dirs_msvc};${_alldest_msvc};${_default_msvc}|${_dirs_mingw};${_alldest_mingw};${_default_mingw})$")
elseif(CMAKE_HOST_SYSTEM_NAME STREQUAL "Darwin")
  set(_bundles [[aaa/bundles;aaa/bundles/testExe3\.app;aaa/bundles/testExe3\.app/Contents;aaa/bundles/testExe3\.app/Contents/Info\.plist;aaa/bundles/testExe3\.app/Contents/MacOS;aaa/bundles/testExe3\.app/Contents/MacOS/testExe3(;aaa/bundles/testExe3\.app/Contents/PkgInfo)?(;aaa/bundles/testExe3\.app/Contents/_CodeSignature;aaa/bundles/testExe3\.app/Contents/_CodeSignature/CodeResources)?]])
  set(_executables [[aaa/executables;aaa/executables/testExe1(-4)?]])
  set(_frameworks [[aaa/frameworks;aaa/frameworks/testLib4\.framework;aaa/frameworks/testLib4\.framework/Headers;aaa/frameworks/testLib4\.framework/Headers/testLib4\.h;aaa/frameworks/testLib4\.framework/Resources;aaa/frameworks/testLib4\.framework/Versions;aaa/frameworks/testLib4\.framework/Versions/A;aaa/frameworks/testLib4\.framework/Versions/A/Resources;aaa/frameworks/testLib4\.framework/Versions/A/Resources/Info\.plist(;aaa/frameworks/testLib4\.framework/Versions/A/_CodeSignature;aaa/frameworks/testLib4\.framework/Versions/A/_CodeSignature/CodeResources)?;aaa/frameworks/testLib4\.framework/Versions/A/testLib4;aaa/frameworks/testLib4\.framework/Versions/Current;aaa/frameworks/testLib4\.framework/testLib4]])
  set(_libraries [[aaa/libraries;aaa/libraries/libtestExe2lib\.dylib;aaa/libraries/libtestLib3lib(-d|-r)?\.1\.2\.dylib;aaa/libraries/libtestLib3lib(-d|-r)?\.3\.dylib;aaa/libraries/libtestMod1\.so;aaa/libraries/testMod2\.bundle;aaa/libraries/testMod2\.bundle/Contents;aaa/libraries/testMod2\.bundle/Contents/Info\.plist;aaa/libraries/testMod2\.bundle/Contents/MacOS;aaa/libraries/testMod2\.bundle/Contents/MacOS/testMod2(;aaa/libraries/testMod2\.bundle/Contents/_CodeSignature;aaa/libraries/testMod2\.bundle/Contents/_CodeSignature/CodeResources)?]])
  set(_dirs "aaa;${_bundles};${_executables};${_frameworks};${_libraries}")

  set(_alldest [[bbb;bbb/libtestExe2lib\.dylib;bbb/libtestLib3lib(-d|-r)?\.1\.2\.dylib;bbb/libtestLib3lib(-d|-r)?\.3\.dylib;bbb/libtestMod1\.so;bbb/testExe1(-4)?;bbb/testExe3\.app;bbb/testExe3\.app/Contents;bbb/testExe3\.app/Contents/Info\.plist;bbb/testExe3\.app/Contents/MacOS;bbb/testExe3\.app/Contents/MacOS/testExe3(;bbb/testExe3\.app/Contents/PkgInfo)?(;bbb/testExe3\.app/Contents/_CodeSignature;bbb/testExe3\.app/Contents/_CodeSignature/CodeResources)?;bbb/testLib4\.framework;bbb/testLib4\.framework/Headers;bbb/testLib4\.framework/Headers/testLib4\.h;bbb/testLib4\.framework/Resources;bbb/testLib4\.framework/Versions;bbb/testLib4\.framework/Versions/A;bbb/testLib4\.framework/Versions/A/Resources;bbb/testLib4\.framework/Versions/A/Resources/Info\.plist(;bbb/testLib4\.framework/Versions/A/_CodeSignature;bbb/testLib4\.framework/Versions/A/_CodeSignature/CodeResources)?;bbb/testLib4\.framework/Versions/A/testLib4;bbb/testLib4\.framework/Versions/Current;bbb/testLib4\.framework/testLib4;bbb/testMod2\.bundle;bbb/testMod2\.bundle/Contents;bbb/testMod2\.bundle/Contents/Info\.plist;bbb/testMod2\.bundle/Contents/MacOS;bbb/testMod2\.bundle/Contents/MacOS/testMod2(;bbb/testMod2\.bundle/Contents/_CodeSignature;bbb/testMod2\.bundle/Contents/_CodeSignature/CodeResources)?]])

  set(_default_bin [[bin;bin/testExe1(-4)?]])
  set(_default_lib [[lib;lib/libtestExe2lib\.dylib;lib/libtestLib3lib(-d|-r)?\.1\.2\.dylib;lib/libtestLib3lib(-d|-r)?\.3\.dylib;lib/libtestMod1\.so;lib/testMod2\.bundle;lib/testMod2\.bundle/Contents;lib/testMod2\.bundle/Contents/Info\.plist;lib/testMod2\.bundle/Contents/MacOS;lib/testMod2\.bundle/Contents/MacOS/testMod2(;lib/testMod2\.bundle/Contents/_CodeSignature;lib/testMod2\.bundle/Contents/_CodeSignature/CodeResources)?]])
  set(_default_bundles [[zzz/bundles;zzz/bundles/testExe3\.app;zzz/bundles/testExe3\.app/Contents;zzz/bundles/testExe3\.app/Contents/Info\.plist;zzz/bundles/testExe3\.app/Contents/MacOS;zzz/bundles/testExe3\.app/Contents/MacOS/testExe3(;zzz/bundles/testExe3\.app/Contents/PkgInfo)?(;zzz/bundles/testExe3\.app/Contents/_CodeSignature;zzz/bundles/testExe3\.app/Contents/_CodeSignature/CodeResources)?]])
  set(_default_frameworks [[zzz/frameworks;zzz/frameworks/testLib4\.framework;zzz/frameworks/testLib4\.framework/Headers;zzz/frameworks/testLib4\.framework/Headers/testLib4\.h;zzz/frameworks/testLib4\.framework/Resources;zzz/frameworks/testLib4\.framework/Versions;zzz/frameworks/testLib4\.framework/Versions/A;zzz/frameworks/testLib4\.framework/Versions/A/Resources;zzz/frameworks/testLib4\.framework/Versions/A/Resources/Info\.plist(;zzz/frameworks/testLib4\.framework/Versions/A/_CodeSignature;zzz/frameworks/testLib4\.framework/Versions/A/_CodeSignature/CodeResources)?;zzz/frameworks/testLib4\.framework/Versions/A/testLib4;zzz/frameworks/testLib4\.framework/Versions/Current;zzz/frameworks/testLib4\.framework/testLib4]])
  set(_default "${_default_bin};${_default_lib};zzz;${_default_bundles};${_default_frameworks}")

  # Need to break this up due to too many pairs of parentheses
  check_installed("^${_dirs};bbb;")
  check_installed(";${_alldest};bin;")
  check_installed(";${_default}$")
endif()
