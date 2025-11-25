
file(GENERATE OUTPUT result.txt CONTENT "$<SOURCE_EXISTS:src,opt,foo>")
