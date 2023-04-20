
file(GENERATE OUTPUT result.txt CONTENT "$<LIST:REMOVE_AT,a;b;c,3>")
