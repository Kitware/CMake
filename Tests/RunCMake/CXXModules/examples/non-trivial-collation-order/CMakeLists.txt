cmake_minimum_required(VERSION 3.24...3.28)
project(cxx_modules_non_trivial_collation_order CXX)

include("${CMAKE_SOURCE_DIR}/../cxx-modules-rules.cmake")

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  include(CheckCompilerFlag)
  check_compiler_flag(CXX "-Wread-modules-implicitly" have_implicit_module_warning)
  if (have_implicit_module_warning)
    add_compile_options(-Werror=read-modules-implicitly)
  endif ()
endif ()

# Returns an random integer from range [0,length)
function(get_random_index result length)
  # math(EXPR) parses and evaluates as 64-bit signed integer.
  string(RANDOM LENGTH 18 ALPHABET "0123456789" value)
  # Modulo is not uniformly distributed, but we doesn't need that here.
  math(EXPR ${result} "${value} % ${length}")
  return(PROPAGATE ${result})
endfunction()

# Shuffle list_var randomly
function(shuffle_list list_var)
  set(result)
  set(length 0)
  foreach(item IN LISTS "${list_var}")
    math(EXPR length "${length} + 1")
    get_random_index(index ${length})
    list(INSERT result ${index} "${item}")
  endforeach()
  set("${list_var}" "${result}" PARENT_SCOPE)
endfunction()

# The import chain is:
# impl-non-partition  --(implicitly)-->  partition_level(primary interface unit)
# --> :intf7 --> :intf6 --> :intf5 --> :intf4 --> :intf3 --> :intf2 --> :intf1
# --> :impl7 --> :impl6 --> :impl5 --> :impl4 --> :impl3 --> :impl2 --> :impl1
set(partition_level_srcs
  partition_level/partition_level.cxx
  partition_level/intf1.cxx
  partition_level/intf3.cxx
  partition_level/intf4.cxx
  partition_level/intf2.cxx   # intentional order
  partition_level/intf5.cxx
  partition_level/intf6.cxx
  partition_level/intf7.cxx
  partition_level/impl1.cxx
  partition_level/impl3.cxx
  partition_level/impl4.cxx
  partition_level/impl2.cxx   # intentional order
  partition_level/impl5.cxx
  partition_level/impl6.cxx
  partition_level/impl7.cxx
)

# The import chain is:
# mod7 --> mod6 --> mod5 --> mod4 --> mod3 --> mod2 --> mod1 --> partition_level
set(module_level_srcs
  ${partition_level_srcs}
  module_level/mod1.cxx
  module_level/mod3.cxx
  module_level/mod4.cxx
  module_level/mod2.cxx   # intentional order
  module_level/mod5.cxx
  module_level/mod6.cxx
  module_level/mod7.cxx
)

# The import chain is:
# target7 --> target6 --> target5 --> target4 -->
# target3 --> target2 --> target1 --> target_module_level
set(targets
  target_module_level
  target1
  target3
  target4
  target2 # intentional order
  target5
  target6
  target7
)

if(non_trivial_collation_order_randomized)
  shuffle_list(module_level_srcs)
  shuffle_list(targets)
endif()

message(STATUS "module_level_srcs: ${module_level_srcs}")
message(STATUS "targets: ${targets}")

foreach(tgt IN LISTS targets)
  if (tgt STREQUAL "target_module_level")

    add_library(target_module_level STATIC)
    target_sources(target_module_level
      PUBLIC
        FILE_SET CXX_MODULES
          BASE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}"
          FILES ${module_level_srcs}
      PRIVATE
        partition_level/impl-non-partition.cxx
    )
    target_compile_features(target_module_level PUBLIC cxx_std_20)

  else()

    add_library(${tgt} STATIC)
    target_sources(${tgt}
      PUBLIC
        FILE_SET CXX_MODULES
          BASE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}"
          FILES target_level/${tgt}.cxx
    )
    target_compile_features(${tgt} PUBLIC cxx_std_20)
    if (tgt MATCHES "^target([2-9])$")
      math(EXPR dep_number "${CMAKE_MATCH_1} - 1")
      target_link_libraries(${tgt} PRIVATE "target${dep_number}")
    else()
      target_link_libraries(${tgt} PRIVATE target_module_level)
    endif()

  endif()
endforeach()

add_executable(exe)
target_link_libraries(exe PRIVATE target7)
target_sources(exe
  PRIVATE
    main.cxx)

add_test(NAME exe COMMAND exe)
