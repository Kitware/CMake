# - This module defines variables and macros required to build eCos application.
# This file contains the following macros:
# ECOS_ADD_INCLUDE_DIRECTORIES() - add the eCos include dirs
# ECOS_ADD_EXECUTABLE(name source1 ... sourceN ) - create an eCos executable
# ECOS_ADJUST_DIRECTORY(VAR source1 ... sourceN ) - adjusts the path of the source files and puts the result into VAR
#
# Macros for selecting the toolchain:
# ECOS_USE_ARM_ELF_TOOLS()       - enable the ARM ELF toolchain for the directory where it is called
# ECOS_USE_I386_ELF_TOOLS()      - enable the i386 ELF toolchain for the directory where it is called
# ECOS_USE_PPC_EABI_TOOLS()      - enable the PowerPC toolchain for the directory where it is called
#
# It contains the following variables:
# ECOS_DEFINITIONS
# ECOSCONFIG_EXECUTABLE
# for internal use only:
#  ECOS_ADD_TARGET_LIB

INCLUDE(AddFileDependencies)


#first check that ecosconfig is available
FIND_PROGRAM(ECOSCONFIG_EXECUTABLE NAMES ecosconfig)
IF(NOT ECOSCONFIG_EXECUTABLE)
   MESSAGE(SEND_ERROR "ecosconfig was not found. Either include it in the system path or set it manually using ccmake.")
ELSE(NOT ECOSCONFIG_EXECUTABLE)
   MESSAGE(STATUS "Found ecosconfig: ${ECOSCONFIG_EXECUTABLE}")
ENDIF(NOT ECOSCONFIG_EXECUTABLE)

#check that ECOS_REPOSITORY is set correctly
IF (NOT EXISTS $ENV{ECOS_REPOSITORY}/ecos.db)
   MESSAGE(SEND_ERROR "The environment variable ECOS_REPOSITORY is not set correctly. Set it to the directory which contains the file ecos.db")
ELSE (NOT EXISTS $ENV{ECOS_REPOSITORY}/ecos.db)
   MESSAGE(STATUS "ECOS_REPOSITORY is set to $ENV{ECOS_REPOSITORY}")
ENDIF (NOT EXISTS $ENV{ECOS_REPOSITORY}/ecos.db)


#add the globale include-diretories
#usage: ECOS_ADD_INCLUDE_DIRECTORIES()
MACRO(ECOS_ADD_INCLUDE_DIRECTORIES)
#check for ProjectSources.txt one level higher
   IF (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/../ProjectSources.txt)
      INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/../)
   ELSE (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/../ProjectSources.txt)
      INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/)
   ENDIF (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/../ProjectSources.txt)

#the ecos include directory
   INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/ecos/install/include/)

ENDMACRO(ECOS_ADD_INCLUDE_DIRECTORIES)


#we want to compile for the xscale processor, in this case the following macro has to be called
#usage: ECOS_USE_ARM_ELF_TOOLS()
MACRO (ECOS_USE_ARM_ELF_TOOLS)
   SET(CMAKE_CXX_COMPILER "arm-elf-c++")
   SET(CMAKE_COMPILER_IS_GNUCXX 1)
   SET(CMAKE_C_COMPILER "arm-elf-gcc")
   SET(CMAKE_AR "arm-elf-ar")
   SET(CMAKE_RANLIB "arm-elf-ranlib")
#for linking
   SET(ECOS_LD_MCPU "-mcpu=xscale")
#for compiling
   ADD_DEFINITIONS(-mcpu=xscale -mapcs-frame)
#for the obj-tools
   SET(ECOS_ARCH_PREFIX "arm-elf-")
ENDMACRO (ECOS_USE_ARM_ELF_TOOLS)

#usage: ECOS_USE_PPC_EABI_TOOLS()
MACRO (ECOS_USE_PPC_EABI_TOOLS)
   SET(CMAKE_CXX_COMPILER "powerpc-eabi-c++")
   SET(CMAKE_COMPILER_IS_GNUCXX 1)
   SET(CMAKE_C_COMPILER "powerpc-eabi-gcc")
   SET(CMAKE_AR "powerpc-eabi-ar")
   SET(CMAKE_RANLIB "powerpc-eabi-ranlib")
#for linking
   SET(ECOS_LD_MCPU "")
#for compiling
   ADD_DEFINITIONS()
#for the obj-tools
   SET(ECOS_ARCH_PREFIX "powerpc-eabi-")
ENDMACRO (ECOS_USE_PPC_EABI_TOOLS)

#usage: ECOS_USE_I386_ELF_TOOLS()
MACRO (ECOS_USE_I386_ELF_TOOLS)
   SET(CMAKE_CXX_COMPILER "i386-elf-c++")
   SET(CMAKE_COMPILER_IS_GNUCXX 1)
   SET(CMAKE_C_COMPILER "i386-elf-gcc")
   SET(CMAKE_AR "i386-elf-ar")
   SET(CMAKE_RANLIB "i386-elf-ranlib")
#for linking
   SET(ECOS_LD_MCPU "")
#for compiling
   ADD_DEFINITIONS()
#for the obj-tools
   SET(ECOS_ARCH_PREFIX "i386-elf-")
ENDMACRO (ECOS_USE_I386_ELF_TOOLS)


#since the actual sources are located one level upwards
#a "../" has to be prepended in front of every source file
#call the following macro to achieve this, the first parameter
#is the name of the new list of source files with adjusted paths,
#followed by all source files
#usage: ECOS_ADJUST_DIRECTORY(adjusted_SRCS ${my_srcs})
MACRO(ECOS_ADJUST_DIRECTORY _target_FILES )
   FOREACH (_current_FILE ${ARGN})
      IF (${_current_FILE} MATCHES "^/.+")  # don't adjust for absolute paths
         SET(tmp ${_current_FILE})
      ELSE (${_current_FILE} MATCHES "^/.+")
         SET(tmp ${CMAKE_CURRENT_SOURCE_DIR}/../${_current_FILE})
         GET_FILENAME_COMPONENT(tmp ${tmp} ABSOLUTE)
      ENDIF (${_current_FILE} MATCHES "^/.+")
      SET(${_target_FILES} ${${_target_FILES}} ${tmp})
   ENDFOREACH (_current_FILE)
ENDMACRO(ECOS_ADJUST_DIRECTORY)

#creates the dependancy from all source files on the ecos target.ld,
#adds the command for compiling ecos and adds target.ld to the make_clean_files
MACRO(ECOS_ADD_TARGET_LIB)
#sources depend on target.ld
    FOREACH (_current_FILE ${ARGN})
      ADD_FILE_DEPENDENCIES(${_current_FILE} ${CMAKE_CURRENT_SOURCE_DIR}/ecos/install/lib/target.ld)
    ENDFOREACH (_current_FILE)

#use a variable for the make_clean_files since later on even more files are added
   SET(ECOS_ADD_MAKE_CLEAN_FILES ${ECOS_ADD_MAKE_CLEAN_FILES};ecos/install/lib/target.ld)
   SET_DIRECTORY_PROPERTIES(
      PROPERTIES
       ADDITIONAL_MAKE_CLEAN_FILES "${ECOS_ADD_MAKE_CLEAN_FILES}" )

   ADD_CUSTOM_COMMAND(OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/ecos/install/lib/target.ld
      COMMAND sh
      ARGS -c \"make -C ecos || exit -1\; if [ -e ecos/install/lib/target.ld ] \; then touch ecos/install/lib/target.ld\; fi\"
      DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/ecos/makefile
   )

   ADD_CUSTOM_COMMAND(OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/ecos/makefile
      COMMAND sh
      ARGS -c \" cd ecos\; ${ECOSCONFIG_EXECUTABLE} tree || exit -1\;\"
      DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/ecos/ecos.ecc
   )

   ADD_CUSTOM_TARGET( ecos make -C ${CMAKE_CURRENT_SOURCE_DIR}/ecos/ DEPENDS  ${CMAKE_CURRENT_SOURCE_DIR}/ecos/makefile )
ENDMACRO(ECOS_ADD_TARGET_LIB)


#macro for creating an executable ecos application
#the first parameter is the name of the executable,
#the second is the list of all source files (where the path
#has been adjusted beforehand by calling ECOS_ADJUST_DIRECTORY()
#usage: ECOS_ADD_EXECUTABLE(my_app ${adjusted_SRCS})
MACRO(ECOS_ADD_EXECUTABLE _exe_NAME )
   #definitions, valid for all ecos projects
   #the optimization and "-g" for debugging has to be enabled
   #in the project-specific CMakeLists.txt
   ADD_DEFINITIONS(-D__ECOS__=1 -D__ECOS=1)
   SET(ECOS_DEFINITIONS -Wall -Wno-long-long -pipe -fno-builtin)

   SET (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wstrict-prototypes")
   SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Woverloaded-virtual -fno-rtti -Wctor-dtor-privacy -fno-strict-aliasing -fno-exceptions")

#the executable depends on ecos target.ld
   ECOS_ADD_TARGET_LIB(${ARGN})

#special link commands for ecos-executables
   SET(CMAKE_CXX_LINK_EXECUTABLE  "<CMAKE_CXX_COMPILER> <CMAKE_CXX_LINK_FLAGS> <OBJECTS>  -o <TARGET> ${_ecos_EXTRA_LIBS} -nostdlib  -nostartfiles -L${CMAKE_CURRENT_SOURCE_DIR}/ecos/install/lib -Ttarget.ld ${ECOS_LD_MCPU}")
   SET(CMAKE_C_LINK_EXECUTABLE  "<CMAKE_C_COMPILER> <CMAKE_C_LINK_FLAGS> <OBJECTS>  -o <TARGET> ${_ecos_EXTRA_LIBS} -nostdlib  -nostartfiles -L${CMAKE_CURRENT_SOURCE_DIR}/ecos/install/lib -Ttarget.ld ${ECOS_LD_MCPU}")

   ADD_EXECUTABLE(${_exe_NAME} ${ARGN})
   SET_TARGET_PROPERTIES(${_exe_NAME} PROPERTIES SUFFIX ".elf")

#create a binary file
   ADD_CUSTOM_COMMAND(
      TARGET ${_exe_NAME}
      POST_BUILD
      COMMAND ${ECOS_ARCH_PREFIX}objcopy
      ARGS -O binary ${_exe_NAME}.elf ${_exe_NAME}.bin
   )

#and an srec file
   ADD_CUSTOM_COMMAND(
      TARGET ${_exe_NAME}
      POST_BUILD
      COMMAND ${ECOS_ARCH_PREFIX}objcopy
      ARGS -O srec ${_exe_NAME}.elf ${_exe_NAME}.srec
   )

#add the created files to the make_clean_files
   SET(ECOS_ADD_MAKE_CLEAN_FILES ${ECOS_ADD_MAKE_CLEAN_FILES};${_exe_NAME}.bin;${_exe_NAME}.srec;${_exe_NAME}.lst;)

   SET_DIRECTORY_PROPERTIES(
      PROPERTIES
       ADDITIONAL_MAKE_CLEAN_FILES "${ECOS_ADD_MAKE_CLEAN_FILES}"
   )

#cd $1; ls -a  | grep --invert-match -e "\(.*CVS\)\|\(.*ecos\.ecc\)\|\(.*\.cvsignore\)\|\(\.\.\?\)" | xargs rm -rf;  touch ecos.ecc
   ADD_CUSTOM_TARGET(ecosclean sh -c \"cd ${CMAKE_CURRENT_SOURCE_DIR}/ecos\; ls -a | grep --invert-match -e \\\"\\\(.*CVS\\\)\\|\\\(.*ecos\\.ecc\\\)\\|\\\(.*\\.cvsignore\\\)\\|\\\(^\\.\\.\\?\\\)\\\" |xargs rm -rf\; touch ecos.ecc \")
   ADD_CUSTOM_TARGET(normalclean ${CMAKE_MAKE_PROGRAM} clean -C ${CMAKE_CURRENT_SOURCE_DIR})
   ADD_DEPENDENCIES (ecosclean normalclean)

   ADD_DEPENDENCIES(ecosclean clean)


   ADD_CUSTOM_TARGET( listing
      COMMAND echo -e   \"\\n--- Symbols sorted by address ---\\n\" > ${_exe_NAME}.lst
      COMMAND ${ECOS_ARCH_PREFIX}nm -S -C -n ${_exe_NAME}.elf >> ${_exe_NAME}.lst
      COMMAND echo -e \"\\n--- Symbols sorted by size ---\\n\" >> ${_exe_NAME}.lst
      COMMAND ${ECOS_ARCH_PREFIX}nm -S -C -r --size-sort ${_exe_NAME}.elf >> ${_exe_NAME}.lst
      COMMAND echo -e \"\\n--- Full assembly listing ---\\n\" >> ${_exe_NAME}.lst
      COMMAND ${ECOS_ARCH_PREFIX}objdump -S -x -d -C ${_exe_NAME}.elf >> ${_exe_NAME}.lst )

ENDMACRO(ECOS_ADD_EXECUTABLE)

