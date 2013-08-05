cmake_minimum_required(VERSION 2.8.3 FATAL_ERROR)
project(CxxFeatures CXX)

find_package(CXXFeatures)

set(_all_cxx_features
        CXXFeatures_auto_FOUND
        CXXFeatures_class_override_final_FOUND
        CXXFeatures_constexpr_FOUND
        CXXFeatures_cstdint_header_FOUND
        CXXFeatures_decltype_FOUND
        CXXFeatures_defaulted_functions_FOUND
        CXXFeatures_delegating_constructors_FOUND
        CXXFeatures_deleted_functions_FOUND
        CXXFeatures_func_identifier_FOUND
        CXXFeatures_initializer_list_FOUND
        CXXFeatures_lambda_FOUND
        CXXFeatures_long_long_FOUND
        CXXFeatures_nullptr_FOUND
        CXXFeatures_rvalue_references_FOUND
        CXXFeatures_sizeof_member_FOUND
        CXXFeatures_static_assert_FOUND
        CXXFeatures_variadic_templates_FOUND
)

unset(_expected_features)
unset(_expected_cxx11_flag)
unset(_compiler_unknown_features)

if (CMAKE_COMPILER_IS_GNUCXX)
    # no idea since when this is supported, but it is at least
    # supported since 3.1
    list(APPEND _expected_features
         CXXFeatures_long_long_FOUND
         CXXFeatures_func_identifier_FOUND)

    if (NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS 4.3)
        set(_expected_cxx11_flag "-std=c++0x")
        if (NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS 4.7)
            set(_expected_cxx11_flag "-std=c++11")
            list(APPEND _expected_features
                 CXXFeatures_class_override_final_FOUND
                 CXXFeatures_delegating_constructors_FOUND)
        endif ()
        if (NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS 4.6)
            list(APPEND _expected_features
                 CXXFeatures_nullptr_FOUND)
        endif ()
        if (NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS 4.5)
            list(APPEND _expected_features
                 CXXFeatures_constexpr_FOUND
                 CXXFeatures_lambda_FOUND)
        endif ()
        if (NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS 4.4)
            list(APPEND _expected_features
                 CXXFeatures_auto_FOUND
                 CXXFeatures_cstdint_header_FOUND
                 CXXFeatures_defaulted_functions_FOUND
                 CXXFeatures_deleted_functions_FOUND
                 CXXFeatures_initializer_list_FOUND
                 CXXFeatures_sizeof_member_FOUND)
        endif ()
        list(APPEND _expected_features
             CXXFeatures_decltype_FOUND
             CXXFeatures_rvalue_references_FOUND
             CXXFeatures_static_assert_FOUND
             CXXFeatures_variadic_templates_FOUND)
    endif ()
elseif (MSVC)
    if (NOT MSVC_VERSION VERSION_LESS 1700)
        list(APPEND _expected_features
             CXXFeatures_class_override_final_FOUND)
    endif ()
    if (NOT MSVC_VERSION VERSION_LESS 1600)
        list(APPEND _expected_features
             CXXFeatures_auto_FOUND
             CXXFeatures_cstdint_header_FOUND
             CXXFeatures_decltype_FOUND
             CXXFeatures_lambda_FOUND
             CXXFeatures_nullptr_FOUND
             CXXFeatures_rvalue_references_FOUND
             CXXFeatures_static_assert_FOUND)
    endif ()
    if (NOT MSVC_VERSION VERSION_LESS 1301)
        list(APPEND _expected_features
             CXXFeatures_long_long_FOUND)
    endif ()
elseif (CMAKE_CXX_COMPILER_ID MATCHES "Borland")
    list(APPEND _expected_features
         CXXFeatures_long_long_FOUND)
elseif (CMAKE_CXX_COMPILER_ID MATCHES "XL")
    if (NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS 10.1)
        set(_expected_cxx11_flag "-qlanglvl=extended0x")
        list(APPEND _expected_features
             CXXFeatures_long_long_FOUND)
    endif ()
    if (NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS 11.1)
        # func_identifier and sizeof_member are here because they were detected on a
        # test machine but it is unclear since when they are supported
        list(APPEND _expected_features
             CXXFeatures_auto_FOUND
             CXXFeatures_decltype_FOUND
             CXXFeatures_delegating_constructors_FOUND
             CXXFeatures_func_identifier_FOUND
             CXXFeatures_sizeof_member_FOUND
             CXXFeatures_static_assert_FOUND
             CXXFeatures_variadic_templates_FOUND)
    endif ()
    if (NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS 12.1)
        list(APPEND _expected_features
             CXXFeatures_constexpr_FOUND
             CXXFeatures_rvalue_references_FOUND)
    endif ()
else ()
    message(WARNING "Your C++ compiler configuration is not in the list of known configurations")
    set(_compiler_unknown_features TRUE)
endif ()

foreach (flag IN LISTS _all_cxx_features)
    list(FIND _expected_features "${flag}" _flag_index)
    if (${flag})
        add_definitions("-D${flag}")
        message(STATUS "Compiler C++ support flag ${flag} set")
        if (_flag_index EQUAL -1 AND NOT _compiler_unknown_features)
            message(SEND_ERROR "C++ feature '${flag}' was detected, but not expected")
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
add_executable(CXXFeatures cxxfeatures.cxx)

enable_testing()
if (NOT CROSS_COMPILING)
    add_test(NAME CXXFeatures COMMAND CXXFeatures)
endif ()
