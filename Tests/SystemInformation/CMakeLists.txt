PROJECT(SystemInformation)
INCLUDE_DIRECTORIES(${SystemInformation_BINARY_DIR})
CONFIGURE_FILE(${SystemInformation_SOURCE_DIR}/SystemInformation.in 
${SystemInformation_BINARY_DIR}/SystemInformation.out)
CONFIGURE_FILE(${SystemInformation_SOURCE_DIR}/DumpInformation.h.in 
${SystemInformation_BINARY_DIR}/DumpInformation.h)
ADD_EXECUTABLE(DumpInformation DumpInformation.cxx)

