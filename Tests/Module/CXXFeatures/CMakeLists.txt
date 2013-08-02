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
    if (NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS 4.7)
        set(_expected_features ${_all_cxx_features})
        set(_expected_cxx11_flag "-std=c++11")
    else ()
        set(_expected_features CXXFeatures_long_long_FOUND)
        set(_expected_cxx11_flag "-std=c++0x")
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

if (NOT CXX11_COMPILER_FLAGS STREQUAL _expected_cxx11_flag)
    message(SEND_ERROR "Found C++11 flag '${CXX11_COMPILER_FLAGS}' but expected '${_expected_cxx11_flag}'")
endif ()
set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} ${CXX11_COMPILER_FLAGS})
add_executable(CXXFeatures cxxfeatures.cxx)

enable_testing()
if (NOT CROSS_COMPILING)
    add_test(NAME CXXFeatures COMMAND CXXFeatures)
endif ()
