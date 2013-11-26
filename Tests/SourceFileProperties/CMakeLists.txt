cmake_minimum_required (VERSION 2.8)

project (SourceFileProperties)



# linking with external would fail because of two main() definitions
set(src ${CMAKE_CURRENT_BINARY_DIR}/src)
file(WRITE ${src}1.cpp "int main() { return 0; }\n")
file(WRITE ${src}2.cpp "int main() { return 0; }\n")

set_source_files_properties(${src}2.cpp PROPERTIES EXTERNAL_SOURCE TRUE)

add_executable(SourceFileProperties ${src}1.cpp ${src}2.cpp)



# build with manual set object file name
file(WRITE ${src}3.cpp "int main() { return 0; }\n")
set(obj ${CMAKE_CURRENT_BINARY_DIR}/object_location${CMAKE_C_OUTPUT_EXTENSION})

set_source_files_properties(${src}3.cpp PROPERTIES OBJECT_LOCATION ${obj})

add_executable(SourceFileProperties2 ${src}3.cpp)



# ensure the defined object filenname was used
set_source_files_properties(external_object PROPERTIES
    EXTERNAL_OBJECT TRUE
    GENERATED TRUE
    OBJECT_LOCATION ${obj}
    LANGUAGE CXX)

add_executable(SourceFileProperties3 external_object)




###########################################
# Precompiled header like tests

set(all_headers ${CMAKE_CURRENT_BINARY_DIR}/all_headers.h)
set(gch ${all_headers}.gch)

file(WRITE ${all_headers} "#include <string>\n")
file(WRITE ${src}4.cpp "std::string foo() { return std::string(); }\n")


if(NOT MSVC)

    set_source_files_properties(${all_headers} PROPERTIES
        HEADER_FILE_ONLY FALSE
        LANGUAGE CXX
        EXTERNAL_SOURCE TRUE
        OBJECT_LOCATION ${gch})

    set_source_files_properties(${src}4.cpp PROPERTIES
        COMPILE_FLAGS "-Winvalid-pch -include ${all_headers}"
        OBJECT_DEPENDS ${gch})

    set(pch_support ${all_headers})

else()

    file(WRITE ${all_headers}.cpp "#include \"${all_headers}\"\n")

    set_source_files_properties(${all_headers}.cpp PROPERTIES
        COMPILE_FLAGS "/Yc\"${all_headers}\" /Fp${gch}"
        OBJECT_OUTPUTS ${gch})

     set_source_files_properties(${src}4.cpp PROPERTIES
        COMPILE_FLAGS "/FI${all_headers} /Yu${all_headers} /Fp${gch}"
        OBJECT_DEPENDS ${gch})

     set(pch_support ${all_headers}.cpp)

endif()

add_library(test_with_pch ${src}4.cpp ${pch_support})
