# IAR C/C++ Compiler (https://www.iar.com)
#
# CPU target architectures (<arch>) supported in CMake:
#           8051, Arm, AVR, MSP430, RH850, RISC-V, RL78, RX, STM8 and V850
#
# IAR C/C++ Compiler for <arch> internal macros used by CMake:
#
# __IAR_COMPILERBASE__
#           Provides a fine-grained compiler internal version,
#           available as integer, and preferred for detection.
#           Format:
#             <__IAR_SYSTEMS_ICC__+1> & 0x00FF0000
#             <major_version>         & 0x0000FF00
#             <minor_version>         & 0x000000FF
#
# __IAR_SYSTEMS_ICC__
#           Provides the compiler internal platform version.
#           In CMake, it is used as a fallback detection option for legacy compilers,
#           when __IAR_COMPILERBASE__ is not available.
#           Format:
#             <iar_platform_version>  & 0x000000FF
#           In use:
#             (<iar_platform_version> << 16) & 0x00FF0000
#
# __ICC<arch>__
#           Provides 1 for the current <arch> in use.
#
# __VER__
#           Provides the compiler version, which is composed differently
#           between Arm and non-Arm target architectures.
#
# __SUBVERSION__
#           Provides the compiler patchlevel version for non-Arm target architectures.
#

set(_compiler_id_pp_test "defined(__IAR_SYSTEMS_ICC__) || defined(__IAR_SYSTEMS_ICC)")

set(_compiler_id_version_compute "
# if defined(__VER__) && defined(__ICCARM__)
#  define @PREFIX@COMPILER_VERSION_MAJOR @MACRO_DEC@((__VER__) / 1000000)
#  define @PREFIX@COMPILER_VERSION_MINOR @MACRO_DEC@(((__VER__) / 1000) % 1000)
#  define @PREFIX@COMPILER_VERSION_PATCH @MACRO_DEC@((__VER__) % 1000)
# elif defined(__VER__) && (defined(__ICCAVR__) || defined(__ICCRX__) || defined(__ICCRH850__) || defined(__ICCRL78__) || defined(__ICC430__) || defined(__ICCRISCV__) || defined(__ICCV850__) || defined(__ICC8051__) || defined(__ICCSTM8__))
#  define @PREFIX@COMPILER_VERSION_MAJOR @MACRO_DEC@((__VER__) / 100)
#  define @PREFIX@COMPILER_VERSION_MINOR @MACRO_DEC@((__VER__) - (((__VER__) / 100)*100))
#  define @PREFIX@COMPILER_VERSION_PATCH @MACRO_DEC@(__SUBVERSION__)
# endif
# if defined(__IAR_COMPILERBASE__)
#  define @PREFIX@COMPILER_VERSION_INTERNAL @MACRO_DEC@(__IAR_COMPILERBASE__)
# else
#  define @PREFIX@COMPILER_VERSION_INTERNAL @MACRO_DEC@((__IAR_SYSTEMS_ICC__ << 16))
# endif")
