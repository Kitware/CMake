
file(GENERATE OUTPUT result.txt CONTENT "$<LIST:FILTER,a;b;c,INCLUDE,^(a>")
