set(ls1 "$<LIST:APPEND,a,b,c:,d>")
set(ls2 "$<LIST:APPEND,a,b,c,:d>")
file(GENERATE OUTPUT LIST-edgecases.txt CONTENT ${ls1}\n${ls2})
