function(my_bad_transform in out)
  # Deliberately does NOT set ${out}
endfunction()

set(mylist alpha bravo charlie)
list(TRANSFORM mylist APPLY my_bad_transform)
