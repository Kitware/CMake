cmake_minimum_required(VERSION 3.10)

# Read mocs_compilation.cpp file into variable
file(READ "${MCF}" MOCS_COMPILATION)
string(REPLACE "\\" "\\\\" MOCS_COMPILATION "${MOCS_COMPILATION}" )
string(REPLACE "\"" "\\\"" MOCS_COMPILATION "${MOCS_COMPILATION}" )
string(REPLACE "\n" "\"\n\"" MOCS_COMPILATION "${MOCS_COMPILATION}" )

# Configure file
configure_file ( "${CF_IN}" "${CF_OUT}" @ONLY )
