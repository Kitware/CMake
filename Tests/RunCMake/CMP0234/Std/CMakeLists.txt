cmake_minimum_required(VERSION 4.5)
project(${RunCMake_TEST} C CXX)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(src_dir ${CMAKE_CURRENT_SOURCE_DIR}/..)

# Add a library for one case of the standard matrix.
#   <policy_value>        OLD or NEW
#   <c_ext>, <cxx_ext>    C_EXTENSIONS and CXX_EXTENSIONS values
#   <c_std>, <cxx_std>    C_STANDARD and CXX_STANDARD values, or empty
# The sources are copied per target because the expectation is set per
# source, and source properties are shared by the directory.
function(cmp0234_case policy_value c_ext cxx_ext c_std cxx_std)
  set(name "CMP0234-${policy_value}-c${c_std}-${c_ext}-cxx${cxx_std}-${cxx_ext}")
  if(policy_value STREQUAL "NEW" AND (NOT c_ext OR NOT cxx_ext))
    # Conformant for both languages.
    set(c_mode STRICT)
    set(cxx_mode STRICT)
  elseif(policy_value STREQUAL "NEW")
    # Extensions are on for both: C is traditional even with a C standard.
    set(c_mode EXTENSIONS)
    set(cxx_mode EXTENSIONS)
  else()
    # Nothing is added: only the C standard option has an effect.
    set(c_mode EXTENSIONS)
    if(c_std)
      set(c_mode STRICT)
    endif()
    set(cxx_mode EXTENSIONS)
  endif()

  configure_file(${src_dir}/main42.c ${name}/main42.c COPYONLY)
  configure_file(${src_dir}/other.cpp ${name}/other.cpp COPYONLY)
  set(c_src ${CMAKE_CURRENT_BINARY_DIR}/${name}/main42.c)
  set(cxx_src ${CMAKE_CURRENT_BINARY_DIR}/${name}/other.cpp)
  # The policy is recorded when a target is created.
  cmake_policy(SET CMP0234 ${policy_value})
  add_library(${name} STATIC ${c_src} ${cxx_src})
  set_property(SOURCE ${c_src} PROPERTY COMPILE_DEFINITIONS EXPECT_${c_mode})
  set_property(SOURCE ${cxx_src} PROPERTY COMPILE_DEFINITIONS EXPECT_${cxx_mode})
  if(c_std)
    set_property(TARGET ${name} PROPERTY C_STANDARD ${c_std})
  endif()
  set_property(TARGET ${name} PROPERTY C_EXTENSIONS ${c_ext})
  if(cxx_std)
    set_property(TARGET ${name} PROPERTY CXX_STANDARD ${cxx_std})
  endif()
  set_property(TARGET ${name} PROPERTY CXX_EXTENSIONS ${cxx_ext})
endfunction()

set(c_stds 11 17)
if(CMAKE_C_STANDARD_LATEST GREATER_EQUAL 23)
  list(APPEND c_stds 23)
endif()
foreach(c_std IN ITEMS "" ${c_stds})
  cmp0234_case(NEW OFF ON "${c_std}" "")
  cmp0234_case(NEW ON ON "${c_std}" "")
  cmp0234_case(OLD OFF ON "${c_std}" "")
endforeach()
foreach(cxx_std IN ITEMS 14 17 20 23)
  cmp0234_case(NEW ON ON "" ${cxx_std})
  cmp0234_case(NEW ON OFF "" ${cxx_std})
  cmp0234_case(OLD ON OFF "" ${cxx_std})
endforeach()
