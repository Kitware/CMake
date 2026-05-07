# Comparator that is correct for most pairs but violates strict weak ordering
# when both elements start with "b".
function(late_violation a b result)
  string(SUBSTRING "${a}" 0 1 a_first)
  string(SUBSTRING "${b}" 0 1 b_first)
  if(a_first STREQUAL "b" AND b_first STREQUAL "b")
    # Violates asymmetry: always returns TRUE for b-vs-b pairs
    set(${result} TRUE PARENT_SCOPE)
  else()
    if("${a}" STRLESS "${b}")
      set(${result} TRUE PARENT_SCOPE)
    else()
      set(${result} FALSE PARENT_SCOPE)
    endif()
  endif()
endfunction()

set(mylist "a" "c" "bb" "ba")
list(SORT mylist COMPARATOR late_violation)
