
enable_language(CXX)

function(check_language_feature_flags lang level)
  if(CMAKE_${lang}${level}_STANDARD_COMPILE_OPTION)
    #this property is an internal implementation detail of CMake
    get_property(known_features GLOBAL PROPERTY CMAKE_${lang}${level}_KNOWN_FEATURES)
    list(LENGTH known_features len)
    if(len LESS 1)
      message(FATAL_ERROR "unable to find known features of ${lang}${level}")
    endif()

    string(TOLOWER ${lang} lang_lower)
    set(known_name ${lang_lower}${level}_known_features)
    set(meta_name  ${lang_lower}${level}_meta_feature)

    add_library(${known_name} STATIC a.cxx)
    target_compile_features(${known_name} PUBLIC ${known_features})
    add_library(${meta_name} STATIC a.cxx)
    target_compile_features(${meta_name} PUBLIC ${lang_lower}_std_${level})
  endif()
endfunction()


check_language_feature_flags(CXX 98)
check_language_feature_flags(CXX 11)
check_language_feature_flags(CXX 14)
