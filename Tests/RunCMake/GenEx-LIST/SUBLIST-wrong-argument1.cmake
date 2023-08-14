
file(GENERATE OUTPUT result.txt CONTENT "$<LIST:SUBLIST,a;b;c,3,-1>")
