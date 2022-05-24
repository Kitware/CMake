set(paren "(")
while(${paren})
  message(STATUS "Condition incorrectly true")
  break()
endwhile()
# FIXME(#23296): The above condition error is tolerated for compatibility.
message(STATUS "Code incorrectly accepted")
