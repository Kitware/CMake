file(READ ${in} predefs)
string(REGEX REPLACE "#define +" "#define CHECK_" predefs "${predefs}")
file(WRITE ${out} "${predefs}")
