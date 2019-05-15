set(CMAKE_CONFIGURATION_TYPES Debug)
enable_language(C)
enable_language(CXX)

add_library(JMC-default-C empty.c)
add_library(JMC-default-CXX empty.cxx)

set(CMAKE_VS_JUST_MY_CODE_DEBUGGING OFF)
add_library(JMC-OFF-C empty.c)
add_library(JMC-OFF-CXX empty.cxx)

set(CMAKE_VS_JUST_MY_CODE_DEBUGGING ON)
add_library(JMC-ON-C empty.c)
add_library(JMC-ON-CXX empty.cxx)

set(CMAKE_VS_JUST_MY_CODE_DEBUGGING OFF)
add_library(JMC-TGT-ON-C empty.c)
set_property(TARGET JMC-TGT-ON-C PROPERTY VS_JUST_MY_CODE_DEBUGGING ON)
add_library(JMC-TGT-ON-CXX empty.cxx)
set_property(TARGET JMC-TGT-ON-CXX PROPERTY VS_JUST_MY_CODE_DEBUGGING ON)
add_library(JMC-TGT-OFF-C empty.c)
set_property(TARGET JMC-TGT-OFF-C PROPERTY VS_JUST_MY_CODE_DEBUGGING OFF)
add_library(JMC-TGT-OFF-CXX empty.cxx)
set_property(TARGET JMC-TGT-OFF-CXX PROPERTY VS_JUST_MY_CODE_DEBUGGING OFF)
