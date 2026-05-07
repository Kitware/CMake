function(bad_comparator a b out)
  # Deliberately does NOT set ${out}
endfunction()

set(mylist c a b)
list(SORT mylist COMPARATOR bad_comparator)
