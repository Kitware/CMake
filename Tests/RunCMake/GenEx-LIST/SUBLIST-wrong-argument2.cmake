
file(GENERATE OUTPUT result.txt CONTENT "$<LIST:SUBLIST,a;b;c,1,-2>")
