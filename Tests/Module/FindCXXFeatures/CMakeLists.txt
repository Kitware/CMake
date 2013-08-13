cmake_minimum_required(VERSION 2.8.3 FATAL_ERROR)
project(FindCXXFeatures CXX)

set(_all_cxx_features
        auto
        class_override_final
        constexpr
        cstdint_header
        decltype
        defaulted_functions
        delegating_constructors
        deleted_functions
        func_identifier
        initializer_list
        lambda
        long_long
        nullptr
        rvalue_references
        sizeof_member
        static_assert
        variadic_templates
)

unset(_expected_features)
unset(_expected_cxx11_flag)
unset(_compiler_unknown_features)

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    # no idea since when this is supported, but it is at least
    # supported since 3.1
    list(APPEND _expected_features
         long_long
         func_identifier)

    if (NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS 4.3)
        set(_expected_cxx11_flag "-std=c++0x")
        if (NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS 4.7)
            set(_expected_cxx11_flag "-std=c++11")
            list(APPEND _expected_features
                 class_override_final
                 delegating_constructors)
        endif ()
        if (NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS 4.6)
            list(APPEND _expected_features
                 nullptr)
        endif ()
        if (NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS 4.5)
            list(APPEND _expected_features
                 constexpr
                 lambda)
        endif ()
        if (NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS 4.4)
            list(APPEND _expected_features
                 auto
                 cstdint_header
                 defaulted_functions
                 deleted_functions
                 initializer_list
                 sizeof_member)
        endif ()
        list(APPEND _expected_features
             decltype
             rvalue_references
             static_assert
             variadic_templates)
    endif ()
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    if (NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS 17)
        list(APPEND _expected_features
             class_override_final)
    endif ()
    if (NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS 16)
        list(APPEND _expected_features
             auto
             cstdint_header
             decltype
             lambda
             nullptr
             rvalue_references
             static_assert)
    endif ()
    if (NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS 13.01)
        list(APPEND _expected_features
             long_long)
    endif ()
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "Borland")
    if (NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS 5.60)
        list(APPEND _expected_features
             long_long)
    endif ()
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "XL")
    if (NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS 10.1)
        set(_expected_cxx11_flag "-qlanglvl=extended0x")
        list(APPEND _expected_features
             long_long)
    endif ()
    if (NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS 11.1)
        # func_identifier and sizeof_member are here because they were detected on a
        # test machine but it is unclear since when they are supported
        list(APPEND _expected_features
             auto
             decltype
             delegating_constructors
             func_identifier
             sizeof_member
             static_assert
             variadic_templates)
    endif ()
    if (NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS 12.1)
        list(APPEND _expected_features
             constexpr
             rvalue_references)
    endif ()
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "SunPro")
    # values found by looking on the test output, may be present much longer
    if (NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS 5.1)
        list(APPEND _expected_features
             long_long
             sizeof_member)
    endif ()
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "PGI")
    # values found by looking on the test output, may be present much longer
    if (NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS 11.3)
        list(APPEND _expected_features
             func_identifier
             long_long)
    endif ()
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "Intel")
    if (NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS 11.1)
        if (WIN32)
            set(_expected_cxx11_flag "/Qstd=c++0x")
        else ()
            set(_expected_cxx11_flag "-std=c++0x")
        endif ()
        list(APPEND _expected_features
             func_identifier
             long_long
             static_assert)
    endif ()
    if (NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS 12)
        list(APPEND _expected_features
             auto
             decltype
             defaulted_functions
             deleted_functions
             func_identifier
             lambda
             long_long
             rvalue_references
             sizeof_member
             static_assert)
    endif ()
    if (NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS 12.1)
        list(APPEND _expected_features
             variadic_templates)
    endif ()
    # constexpr is partially supported in version 13, it may
    # already show up for this version depending on the check file.
    # The same applies to initializer lists.
    if (NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS 13)
        if (WIN32)
            set(_expected_cxx11_flag "/Qstd=c++11")
        else ()
            set(_expected_cxx11_flag "-std=c++11")
        endif ()
        list(APPEND _expected_features
             nullptr)
    endif ()
    if (NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS 14)
        list(APPEND _expected_features
             class_override_final
             constexpr
             delegating_constructors
             initializer_list)
    endif ()
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "Watcom")
    # values found by looking on the test output, may be present much longer
    if (NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS 12.70)
        list(APPEND _expected_features
             func_identifier
             long_long)
    endif ()
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MIPSpro")
    # values found by looking on the test output, may be present much longer
    if (NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS 7.4.4)
        list(APPEND _expected_features
             func_identifier
             long_long)
    endif ()
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "HP")
    # values found by looking on the test output, may be present much longer
    if (NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS 6.20)
        list(APPEND _expected_features
             func_identifier
             long_long)
    endif ()
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    if (NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS 2.9)
         set(_expected_cxx11_flag "-std=c++0x")
        list(APPEND _expected_features
             auto
             class_override_final
             constexpr
             decltype
             deleted_functions
             func_identifier
             long_long
             rvalue_references
             sizeof_member
             static_assert
             variadic_templates)
    endif ()
    if (NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS 3)
          # FIXME: I've seen both :( FreeBSD 9.0 has 0x, 10.0 has 11
         set(_expected_cxx11_flag "-std=c++11")
         list(APPEND _expected_features
             defaulted_functions
             delegating_constructors)
    endif ()
    if (NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS 3.1)
         set(_expected_cxx11_flag "-std=c++11")
         list(APPEND _expected_features
             cstdint_header
             initializer_list
             lambda
             nullptr)
    endif ()

else ()
    message(STATUS "CTEST_FULL_OUTPUT")
    message(WARNING "Your C++ compiler configuration is not in the list of known configurations")
    set(_compiler_unknown_features TRUE)
endif ()

find_package(CXXFeatures)

foreach (flag IN LISTS _all_cxx_features)
    list(FIND _expected_features "${flag}" _flag_index)
    set(_full_flag CXXFeatures_${flag}_FOUND)
    if (${_full_flag})
        add_definitions("-D${_full_flag}")
        message(STATUS "Compiler C++ support flag ${_full_flag} set")
        if (_flag_index EQUAL -1 AND NOT _compiler_unknown_features)
            message(WARNING "C++ feature '${flag}' was detected, but not expected")
        endif ()
    else ()
        if (NOT _flag_index EQUAL -1)
            message(SEND_ERROR "Expected C++ feature '${flag}' not detected")
        endif ()
    endif ()
endforeach (flag)

# variables must be expanded here so it still works if both are empty
if (NOT "${CXX11_COMPILER_FLAGS}" STREQUAL "${_expected_cxx11_flag}" AND NOT _compiler_unknown_features)
    message(SEND_ERROR "Found C++11 flag '${CXX11_COMPILER_FLAGS}' but expected '${_expected_cxx11_flag}'")
endif ()
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CXX11_COMPILER_FLAGS}")
add_executable(FindCXXFeatures cxxfeatures.cxx)

enable_testing()
if (NOT CROSS_COMPILING)
    add_test(NAME FindCXXFeatures COMMAND FindCXXFeatures)
endif ()
