enable_language(C CXX)

get_property(enabled_languages GLOBAL PROPERTY ENABLED_LANGUAGES)
list(REMOVE_ITEM enabled_languages NONE)
list(JOIN enabled_languages "," enabled_languages_genex)
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/enabled-languages.txt" "${enabled_languages}")

add_library(concrete STATIC empty.c)
add_library(headers INTERFACE)
add_library(imported INTERFACE IMPORTED)
set_property(TARGET imported PROPERTY SYSTEM FALSE)
target_link_libraries(concrete PRIVATE headers imported)

set(include_root "${CMAKE_CURRENT_BINARY_DIR}/include")
foreach(dir IN ITEMS common c-only cxx-only cxx-id
    system-C system-CXX system-cxx-id all-languages mixed system-common)
  file(MAKE_DIRECTORY "${include_root}/${dir}")
endforeach()
foreach(language IN LISTS enabled_languages)
  file(MAKE_DIRECTORY "${include_root}/lang-${language}")
endforeach()

foreach(target IN ITEMS concrete headers imported)
  target_include_directories(${target} INTERFACE
    "${include_root}/common"
    # Visual Studio also evaluates includes for MIDL, exclude from include_directories.
    "$<$<COMPILE_LANGUAGE:${enabled_languages_genex}>:${include_root}/lang-$<COMPILE_LANGUAGE>>"
    "$<$<COMPILE_LANGUAGE:C>:${include_root}/c-only>"
    "$<$<COMPILE_LANGUAGE:CXX>:${include_root}/cxx-only>"
    "$<$<COMPILE_LANG_AND_ID:CXX,${CMAKE_CXX_COMPILER_ID}>:${include_root}/cxx-id>"
    "$<$<COMPILE_LANG_AND_ID:CXX,NonexistentCompiler>:${include_root}/wrong-id>"
    "$<$<COMPILE_LANGUAGE:C,CXX>:${include_root}/all-languages>"
    # Repeating a path must not duplicate entries or their language lists.
    "$<$<COMPILE_LANGUAGE:CXX>:${include_root}/cxx-only>"
    "${include_root}/mixed"
    "$<INSTALL_INTERFACE:unused>")
  target_include_directories(${target} SYSTEM INTERFACE
    "${include_root}/system-common"
    "$<$<COMPILE_LANGUAGE:C>:${include_root}/system-C>"
    "$<$<COMPILE_LANGUAGE:CXX>:${include_root}/system-CXX>"
    "$<$<COMPILE_LANG_AND_ID:CXX,${CMAKE_CXX_COMPILER_ID}>:${include_root}/system-cxx-id>"
    "$<$<COMPILE_LANG_AND_ID:CXX,NonexistentCompiler>:${include_root}/system-wrong-id>"
    # This path is a regular include for C and a system include for C++.
    "$<$<COMPILE_LANGUAGE:CXX>:${include_root}/mixed>")
endforeach()
