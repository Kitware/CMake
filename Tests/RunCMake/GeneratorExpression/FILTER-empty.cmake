cmake_policy(VERSION 3.11)

file(GENERATE OUTPUT "FILTER-generated.txt" CONTENT "$<FILTER:,INCLUDE,>")
