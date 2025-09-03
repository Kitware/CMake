include_guard()

function(setup_skip_linter_test lang state)
  if(lang STREQUAL "CXX")
    set(maybe_genex_pre "$<1:")
    set(maybe_genex_post ">")
  endif()

  set(CMAKE_${lang}_INCLUDE_WHAT_YOU_USE "${maybe_genex_pre}${PSEUDO_IWYU}${maybe_genex_post}" -some -args)
  set(CMAKE_${lang}_CLANG_TIDY "${maybe_genex_pre}${PSEUDO_TIDY}${maybe_genex_post}" -bad)
  set(CMAKE_${lang}_CPPLINT "${maybe_genex_pre}${PSEUDO_CPPLINT}${maybe_genex_post}" --error)
  set(CMAKE_${lang}_CPPCHECK "${maybe_genex_pre}${PSEUDO_CPPCHECK}${maybe_genex_post}" -error)

  string(TOLOWER "${lang}" ext)
  add_executable(main main.${ext})
  set_source_files_properties(main.${ext} PROPERTIES SKIP_LINTING ${state})
endfunction()
