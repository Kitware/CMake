function(reflexive_comparator a b out)
  set(${out} TRUE PARENT_SCOPE)
endfunction()

set(mylist c a b)
list(SORT mylist COMPARATOR reflexive_comparator)
