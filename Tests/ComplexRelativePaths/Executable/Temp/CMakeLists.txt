#
# Output the files required by 'complex' to a file.
# The 'complex' executable will then test if this file exists and remove it.
# The contents of this file is not tested (absolute paths).
#
OUTPUT_REQUIRED_FILES(
   ${Complex_SOURCE_DIR}/Executable/complex.cxx 
   ${Complex_BINARY_DIR}/Executable/Temp/complex-required.txt)  
