cmake_minimum_required(VERSION 3.21)
project(InterfaceLinkLibrariesDirect C)

include(testStaticLibPlugin.cmake)
add_executable(InterfaceLinkLibrariesDirect main.c)
target_link_libraries(InterfaceLinkLibrariesDirect PRIVATE testStaticLibWithPlugin)

include(testSharedLibWithHelper.cmake)
add_executable(UseSharedLibWithHelper UseSharedLibWithHelper.c)
target_link_libraries(UseSharedLibWithHelper PRIVATE testSharedLibWithHelper testSharedLibHelperExclude)

include(testExeWithPluginHelper.cmake)
add_library(ExePlugin MODULE ExePlugin.c)
target_link_libraries(ExePlugin PRIVATE testExeWithPluginHelper testExePluginHelperExclude)

#----------------------------------------------------------------------------

# Offer usage requirements and symbols to be used through static libs below.
add_library(A STATIC
  a_always.c

  # Good symbols that direct_from_A libraries poison if incorrectly used.
  a_not_direct_from_A.c
  a_not_direct_from_A_for_exe.c
  a_not_direct_from_A_optional.c

  # Bad symbols in direct_from_A libraries below to ensure they come first.
  a_poison_direct_from_A.c
  a_poison_direct_from_A_for_exe.c
  a_poison_direct_from_A_optional.c
  )

# Propagates as usage requirement from A.
add_library(direct_from_A STATIC direct_from_A.c direct_from_A_poison.c)
target_compile_definitions(direct_from_A INTERFACE DEF_DIRECT_FROM_A)
set_property(TARGET A APPEND PROPERTY INTERFACE_LINK_LIBRARIES_DIRECT direct_from_A)

# Propagates as usage requirement from A, but only for executables.
add_library(direct_from_A_for_exe STATIC direct_from_A_for_exe.c direct_from_A_for_exe_poison.c)
target_compile_definitions(direct_from_A_for_exe INTERFACE DEF_DIRECT_FROM_A_FOR_EXE)
set_property(TARGET A APPEND PROPERTY INTERFACE_LINK_LIBRARIES_DIRECT
  "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:direct_from_A_for_exe>")

# Propagates as usage requirement from A, but only for targets that opt-in.
add_library(direct_from_A_optional STATIC direct_from_A_optional.c direct_from_A_optional_poison.c)
target_compile_definitions(direct_from_A_optional INTERFACE DEF_DIRECT_FROM_A_OPTIONAL)
set_property(TARGET A APPEND PROPERTY INTERFACE_LINK_LIBRARIES_DIRECT
  "$<$<BOOL:$<TARGET_PROPERTY:A_LINK_OPTIONAL>>:direct_from_A_optional>")

# Uses and propagates A's usage requirements.
# Does not use the exe-only or optional usage requirements.
add_library(static_A_public STATIC static_A_public.c)
target_link_libraries(static_A_public PUBLIC A)

# Uses A's usage requirements, but propagates only the linking requirements.
# Does not use the exe-only usage requirement.  Does use the optional one.
add_library(static_A_private STATIC static_A_private.c)
target_link_libraries(static_A_private PRIVATE A)
set_property(TARGET static_A_private PROPERTY A_LINK_OPTIONAL 1)

# Uses A's usage requirements, including an optional one.
add_executable(exe_use_static_A_public exe_use_static_A_public.c)
target_link_libraries(exe_use_static_A_public PRIVATE static_A_public)
set_property(TARGET exe_use_static_A_public PROPERTY A_LINK_OPTIONAL 1)

# Does not use A's usage requirements, but does use its non-optional linking requirements.
add_executable(exe_use_static_A_private exe_use_static_A_private.c)
target_link_libraries(exe_use_static_A_private PRIVATE static_A_private)

# Uses A's usage requirements, including an optional one, but overrides the link ordering.
add_executable(exe_use_static_A_public_explicit exe_use_static_A_public_explicit.c)
target_link_libraries(exe_use_static_A_public_explicit PRIVATE static_A_public A direct_from_A direct_from_A_for_exe direct_from_A_optional)
set_property(TARGET exe_use_static_A_public_explicit PROPERTY A_LINK_OPTIONAL 1)

#----------------------------------------------------------------------------

# Test how original and injected dependencies get ordered.

# A bunch of static libraries that need to be linked in alphabetic order.
# Each library has an extra source to poison all symbols meant to be
# provided by earlier libraries.  This enforces ordering on platforms
# whose linkers re-visit static libraries.
add_library(order_A STATIC order_A.c)
add_library(order_B STATIC order_B.c order_B_poison.c)
add_library(order_C STATIC order_C.c order_C_poison.c)
add_library(order_D STATIC order_D.c order_D_poison.c)
add_library(order_E STATIC order_E.c order_E_poison.c)
add_library(order_F STATIC order_F.c order_F_poison.c)
add_library(order_G STATIC order_G.c order_G_poison.c)
add_library(order_H STATIC order_H.c order_H_poison.c)
add_library(order_I STATIC order_I.c order_I_poison.c)
add_library(order_J STATIC order_J.c order_J_poison.c)

# An executable to drive linking.
add_executable(order_main order_main.c)

# In the following diagram, connection by a slash means the top
# target lists the bottom target in a link interface property:
#
#  \ => INTERFACE_LINK_LIBRARIES
#  / => INTERFACE_LINK_LIBRARIES_DIRECT
#
# The top of each tree represents an entry in the exe's LINK_LIBRARIES.
# CMake should evaluate this graph to generate the proper link order.
#
#             D        H
#            / \      / \
#           B   J    F   I
#          /   /    /   / \
#         A   C    E   G   J
set_property(TARGET order_main PROPERTY LINK_LIBRARIES order_D order_H)
set_property(TARGET order_D PROPERTY INTERFACE_LINK_LIBRARIES_DIRECT order_B)
set_property(TARGET order_D PROPERTY INTERFACE_LINK_LIBRARIES order_J)
set_property(TARGET order_B PROPERTY INTERFACE_LINK_LIBRARIES_DIRECT order_A)
set_property(TARGET order_J PROPERTY INTERFACE_LINK_LIBRARIES_DIRECT order_C)
set_property(TARGET order_H PROPERTY INTERFACE_LINK_LIBRARIES_DIRECT order_F)
set_property(TARGET order_H PROPERTY INTERFACE_LINK_LIBRARIES order_I)
set_property(TARGET order_F PROPERTY INTERFACE_LINK_LIBRARIES_DIRECT order_E)
set_property(TARGET order_I PROPERTY INTERFACE_LINK_LIBRARIES_DIRECT order_G)
set_property(TARGET order_I PROPERTY INTERFACE_LINK_LIBRARIES order_J)

#----------------------------------------------------------------------------

# Test that the original LINK_LIBRARIES cannot be re-ordered by injection
# from usage requirements.

# A bunch of static libraries that need to be linked in alphabetic order.
# Each library has an extra source to poison all symbols meant to be
# provided by earlier libraries.  This enforces ordering on platforms
# whose linkers re-visit static libraries.
add_library(force_A STATIC order_A.c)
add_library(force_B STATIC order_B.c order_B_poison.c)
add_library(force_C STATIC order_C.c order_C_poison.c)
add_library(force_D STATIC order_D.c order_D_poison.c)
add_library(force_E STATIC order_E.c order_E_poison.c)
add_library(force_F STATIC order_F.c order_F_poison.c)
add_library(force_G STATIC order_G.c order_G_poison.c)
add_library(force_H STATIC order_H.c order_H_poison.c)
add_library(force_I STATIC order_I.c order_I_poison.c)
add_library(force_J STATIC order_J.c order_J_poison.c)

# An executable to drive linking.
add_executable(force_main order_main.c)

# The executable explicitly lists all the libraries in the right order.
target_link_libraries(force_main PRIVATE force_A force_B force_C force_D force_E force_F force_G force_H)

# Add legitimate normal dependencies.
set_property(TARGET force_D PROPERTY INTERFACE_LINK_LIBRARIES force_J)
set_property(TARGET force_H PROPERTY INTERFACE_LINK_LIBRARIES force_I)
set_property(TARGET force_I PROPERTY INTERFACE_LINK_LIBRARIES force_J)

# Add bogus injected direct dependencies to verify that they do not
# change the original order of LINK_LIBRARIES.
set_property(TARGET force_A PROPERTY INTERFACE_LINK_LIBRARIES_DIRECT force_B)
set_property(TARGET force_B PROPERTY INTERFACE_LINK_LIBRARIES_DIRECT force_C)
set_property(TARGET force_C PROPERTY INTERFACE_LINK_LIBRARIES_DIRECT force_D)
set_property(TARGET force_D PROPERTY INTERFACE_LINK_LIBRARIES_DIRECT force_E)
set_property(TARGET force_E PROPERTY INTERFACE_LINK_LIBRARIES_DIRECT force_F)
set_property(TARGET force_F PROPERTY INTERFACE_LINK_LIBRARIES_DIRECT force_G)
set_property(TARGET force_G PROPERTY INTERFACE_LINK_LIBRARIES_DIRECT force_H)
