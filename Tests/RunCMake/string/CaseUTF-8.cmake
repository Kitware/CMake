#                 Chinese Hindi  Greek English Russian
set(string_mixed "注意    यूनिकोड είναι VeRy    здорово!") # UTF-8
set(string_upper "注意    यूनिकोड είναι VERY    здорово!") # UTF-8
set(string_lower "注意    यूनिकोड είναι very    здорово!") # UTF-8
string(TOLOWER "${string_mixed}" output_lower)
string(TOUPPER "${string_mixed}" output_upper)
foreach(case lower upper)
  if(NOT "${output_${case}}" STREQUAL "${string_${case}}")
    message(SEND_ERROR "to${case} failed:\n  \"${output_${case}}\"")
  endif()
endforeach()
