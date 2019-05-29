# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

if(UNIX)
  set(CMAKE_Swift_OUTPUT_EXTENSION .o)
else()
  set(CMAKE_Swift_OUTPUT_EXTENSION .obj)
endif()

# Load compiler-specific information.
if(CMAKE_Swift_COMPILER_ID)
  include(Compiler/${CMAKE_Swift_COMPILER_ID}-Swift OPTIONAL)

  if(CMAKE_SYSTEM_PROCESSOR)
    include(Platform/${CMAKE_EFFECTIVE_SYSTEM_NAME}-${CMAKE_Swift_COMPILER_ID}-Swift-${CMAKE_SYSTEM_PROCESSOR} OPTIONAL)
  endif()
  include(Platform/${CMAKE_EFFECTIVE_SYSTEM_NAME}-${CMAKE_Swift_COMPILER_ID}-Swift OPTIONAL)
endif()

set(CMAKE_INCLUDE_FLAG_Swift "-I ")

set(CMAKE_Swift_COMPILE_OPTIONS_TARGET "-target ")
set(CMAKE_Swift_COMPILER_ARG1 -frontend)
set(CMAKE_Swift_DEFINE_FLAG -D)
set(CMAKE_Swift_FRAMEWORK_SEARCH_FLAG "-F ")
set(CMAKE_Swift_LINKER_WRAPPER_FLAG "-Xlinker" " ")
set(CMAKE_Swift_RESPONSE_FILE_LINK_FLAG @)

set(CMAKE_Swift_LINKER_PREFERENCE 50)
set(CMAKE_Swift_LINKER_PREFERENCE_PROPAGATES 1)

# NOTE(compnerd) use the short form for convenience and ease of search.  They
# are treated equivalent to their long form names as well as custom Swift
# specific names.
set(CMAKE_Swift_COMPILE_OPTIONS_MSVC_RUNTIME_LIBRARY_MultiThreaded -libc MT)
set(CMAKE_Swift_COMPILE_OPTIONS_MSVC_RUNTIME_LIBRARY_MultiThreadedDLL -libc MD)
set(CMAKE_Swift_COMPILE_OPTIONS_MSVC_RUNTIME_LIBRARY_MultiThreadedDebug -libc MTd)
set(CMAKE_Swift_COMPILE_OPTIONS_MSVC_RUNTIME_LIBRARY_MultiThreadedDebugDLL -libc MDd)

set(CMAKE_Swift_FLAGS_DEBUG_INIT "-g")
set(CMAKE_Swift_FLAGS_RELEASE_INIT "-O")
set(CMAKE_Swift_FLAGS_RELWITHDEBINFO_INIT "-O -g")
set(CMAKE_Swift_FLAGS_MINSIZEREL_INIT "-Osize")

# NOTE(compnerd) we do not have an object compile rule since we build the objects as part of the link step
if(NOT CMAKE_Swift_COMPILE_OBJECT)
  set(CMAKE_Swift_COMPILE_OBJECT ":")
endif()

if(NOT CMAKE_Swift_CREATE_SHARED_LIBRARY)
  if(CMAKE_Swift_COMPILER_TARGET)
    set(CMAKE_Swift_CREATE_SHARED_LIBRARY "${CMAKE_Swift_COMPILER} -target <CMAKE_Swift_COMPILER_TARGET> -output-file-map <SWIFT_OUTPUT_FILE_MAP> -incremental -emit-library -o <TARGET> -module-name <SWIFT_MODULE_NAME> -module-link-name <SWIFT_LIBRARY_NAME> -emit-module -emit-module-path <SWIFT_MODULE> -emit-dependencies <DEFINES> <FLAGS> <INCLUDES> <SWIFT_SOURCES> <LINK_FLAGS> <LINK_LIBRARIES>")
  else()
    set(CMAKE_Swift_CREATE_SHARED_LIBRARY "${CMAKE_Swift_COMPILER} -output-file-map <SWIFT_OUTPUT_FILE_MAP> -incremental -emit-library -o <TARGET> -module-name <SWIFT_MODULE_NAME> -module-link-name <SWIFT_LIBRARY_NAME> -emit-module -emit-module-path <SWIFT_MODULE> -emit-dependencies <DEFINES> <FLAGS> <INCLUDES> <SWIFT_SOURCES> <LINK_FLAGS> <LINK_LIBRARIES>")
  endif()
endif()

if(NOT CMAKE_Swift_CREATE_SHARED_MODULE)
  set(CMAKE_Swift_CREATE_SHARED_MODULE ${CMAKE_Swift_CREATE_SHARED_LIBRARY})
endif()

if(NOT CMAKE_Swift_LINK_EXECUTABLE)
  if(CMAKE_Swift_COMPILER_TARGET)
    set(CMAKE_Swift_LINK_EXECUTABLE "${CMAKE_Swift_COMPILER} -target <CMAKE_Swift_COMPILER_TARGET> -output-file-map <SWIFT_OUTPUT_FILE_MAP> -incremental -emit-executable -o <TARGET> -emit-module -emit-module-path <SWIFT_MODULE> -emit-dependencies <DEFINES> <FLAGS> <INCLUDES> <SWIFT_SOURCES> <LINK_FLAGS> <LINK_LIBRARIES>")
  else()
    set(CMAKE_Swift_LINK_EXECUTABLE "${CMAKE_Swift_COMPILER} -output-file-map <SWIFT_OUTPUT_FILE_MAP> -incremental -emit-executable -o <TARGET> -emit-module -emit-module-path <SWIFT_MODULE> -emit-dependencies <DEFINES> <FLAGS> <INCLUDES> <SWIFT_SOURCES> <LINK_FLAGS> <LINK_LIBRARIES>")
  endif()
endif()

if(NOT CMAKE_Swift_CREATE_STATIC_LIBRARY)
  set(CMAKE_Swift_ARCHIVE_CREATE "<CMAKE_AR> crs <TARGET> <OBJECTS>")
  set(CMAKE_Swift_ARCHIVE_FINISH "")
endif()

set(CMAKE_Swift_INFORMATION_LOADED 1)
