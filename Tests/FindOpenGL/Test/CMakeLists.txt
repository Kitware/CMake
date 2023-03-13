cmake_minimum_required(VERSION 3.10)
cmake_policy(SET CMP0072 NEW)
project(TestFindOpenGL C)
include(CTest)

find_package(OpenGL REQUIRED)

# import target for GLU
add_executable(test_tgt main.c)
target_link_libraries(test_tgt OpenGL::GLU)
add_test(NAME test_tgt COMMAND test_tgt)

# OPENGL_LIBRARIES should be whatever libraries are needed to link.
add_executable(test_var main.c)
target_include_directories(test_var PRIVATE ${OPENGL_INGLUDE_DIRS})
target_link_libraries(test_var PRIVATE ${OPENGL_LIBRARIES})
add_test(NAME test_var COMMAND test_var)

# VND support adds an ::OpenGL import target.  This can be used for OpenGL-only
# code (code that does not manipulate contexts, like our 'main.c').  Without
# VND, ::GL can be used for both context and non-context OpenGL code.
if(OpenGL_TEST_VND)
  add_executable(test_comp_none main.c)
  target_link_libraries(test_comp_none PRIVATE OpenGL::OpenGL)
  add_test(NAME test_comp_none COMMAND test_comp_none)
else()
  add_executable(test_comp_none main.c)
  target_link_libraries(test_comp_none PRIVATE OpenGL::GL)
  add_test(NAME test_comp_none COMMAND test_comp_none)
endif()

# GLX
if(OpenGL_TEST_VND)
  find_package(OpenGL REQUIRED COMPONENTS OpenGL GLX)
  add_executable(test_comp_glx main.c)
  target_link_libraries(test_comp_glx PRIVATE OpenGL::OpenGL OpenGL::GLX)
  add_test(NAME test_comp_glx COMMAND test_comp_glx)
else()
  # non-VND systems won't have it, but an optional search for GLX should still
  # be okay.
  find_package(OpenGL COMPONENTS GLX)
  add_executable(test_comp_glx_novnd main.c)
  target_link_libraries(test_comp_glx_novnd PRIVATE OpenGL::GL)
  add_test(NAME test_comp_glx_novnd COMMAND test_comp_glx_novnd)
endif()

find_package(OpenGL COMPONENTS OpenGL EGL)
if(OpenGL_EGL_FOUND)
  add_executable(test_comp_egl main.c)
  target_link_libraries(test_comp_egl PRIVATE OpenGL::OpenGL OpenGL::EGL)
  add_test(NAME test_comp_egl COMMAND test_comp_egl)
  # EGL-only code should not link to GLX.
  get_target_property(iface_libs OpenGL::EGL INTERFACE_LINK_LIBRARIES)
  if(iface_libs MATCHES "GLX")
    message(FATAL_ERROR "EGL-only code links to GLX!")
  endif()
endif()

# all three COMPONENTS together.
find_package(OpenGL COMPONENTS OpenGL EGL GLX)
if(OpenGL_EGL_FOUND AND OpenGL_GLX_FOUND)
  add_executable(test_comp_both main.c)
  target_link_libraries(test_comp_both PRIVATE OpenGL::OpenGL OpenGL::EGL
                        OpenGL::GLX)
  add_test(NAME test_comp_both COMMAND test_comp_both)
endif()

find_package(OpenGL COMPONENTS GLES2)
if(OpenGL_GLES2_FOUND)
  add_executable(test_comp_gles2 main_gles2.c)
  target_link_libraries(test_comp_gles2 PRIVATE OpenGL::GLES2)
  add_test(NAME test_comp_gles2 COMMAND test_comp_gles2)
  # GLES2-only code should not link to OpenGL
  get_target_property(iface_libs test_comp_gles2 LINK_LIBRARIES)
  if(iface_libs MATCHES "OpenGL::OpenGL")
    message(FATAL_ERROR "GLES2-only code links to OpenGL!")
  endif()
endif()

# GLES2 and EGL together.
find_package(OpenGL COMPONENTS GLES2 EGL)
if(OpenGL_GLES2_FOUND AND OpenGL_EGL_FOUND)
  add_executable(test_comp_gles2_egl main_gles2.c)
  target_link_libraries(test_comp_gles2_egl PRIVATE OpenGL::GLES2
                        OpenGL::EGL)
  add_test(NAME test_comp_gles2_egl COMMAND test_comp_gles2_egl)
  # GLES2-EGL-only code should not link to OpenGL or GLX
  get_target_property(iface_libs test_comp_gles2_egl LINK_LIBRARIES)
  if(iface_libs MATCHES "OpenGL::OpenGL")
    message(FATAL_ERROR "GLES2-only code links to OpenGL!")
  endif()
  if(iface_libs MATCHES "GLX")
    message(FATAL_ERROR "GLES2-EGL-only code links to GLX!")
  endif()
endif()

# GLES2 and GLX together.
find_package(OpenGL COMPONENTS GLES2 GLX)
if(OpenGL_GLES2_FOUND AND OpenGL_GLX_FOUND)
  add_executable(test_comp_gles2_glx main_gles2.c)
  target_link_libraries(test_comp_gles2_glx PRIVATE OpenGL::GLES2
                        OpenGL::GLX)
  add_test(NAME test_comp_gles2_glx COMMAND test_comp_gles2_glx)
  # GLES2-GLX-only code should not link to OpenGL or EGL
  get_target_property(iface_libs test_comp_gles2_glx LINK_LIBRARIES)
  if(iface_libs MATCHES "OpenGL::OpenGL")
    message(FATAL_ERROR "GLES2-only code links to OpenGL!")
  endif()
  if(iface_libs MATCHES "EGL")
    message(FATAL_ERROR "GLES2-GLX-only code links to EGL!")
  endif()
endif()

find_package(OpenGL COMPONENTS GLES3)
if(OpenGL_GLES3_FOUND)
  add_executable(test_comp_gles3 main_gles3.c)
  target_link_libraries(test_comp_gles3 PRIVATE OpenGL::GLES3)
  add_test(NAME test_comp_gles3 COMMAND test_comp_gles3)
  # GLES3-only code should not link to OpenGL.
  get_target_property(iface_libs test_comp_gles3 LINK_LIBRARIES)
  if(iface_libs MATCHES "OpenGL::OpenGL")
    message(FATAL_ERROR "GLES3-only code links to OpenGL!")
  endif()
endif()

# GLES3 and EGL together.
find_package(OpenGL COMPONENTS GLES3 EGL)
if(OpenGL_GLES3_FOUND AND OpenGL_EGL_FOUND)
  add_executable(test_comp_gles3_egl main_gles3.c)
  target_link_libraries(test_comp_gles3_egl PRIVATE OpenGL::GLES3
                        OpenGL::EGL)
  add_test(NAME test_comp_gles3_egl COMMAND test_comp_gles3_egl)
  # GLES3-EGL-only code should not link to OpenGL or GLX
  get_target_property(iface_libs test_comp_gles3_egl LINK_LIBRARIES)
  if(iface_libs MATCHES "OpenGL::OpenGL")
    message(FATAL_ERROR "GLES3-only code links to OpenGL!")
  endif()
  if(iface_libs MATCHES "GLX")
    message(FATAL_ERROR "GLES3-EGL-only code links to GLX!")
  endif()
endif()

# GLES3 and GLX together.
find_package(OpenGL COMPONENTS GLES3 GLX)
if(OpenGL_GLES3_FOUND AND OpenGL_GLX_FOUND)
  add_executable(test_comp_gles3_glx main_gles3.c)
  target_link_libraries(test_comp_gles3_glx PRIVATE OpenGL::GLES3
                        OpenGL::GLX)
  add_test(NAME test_comp_gles3_glx COMMAND test_comp_gles3_glx)
  # GLESr-GLX-only code should not link to OpenGL or EGL
  get_target_property(iface_libs test_comp_gles3_glx LINK_LIBRARIES)
  if(iface_libs MATCHES "OpenGL::OpenGL")
    message(FATAL_ERROR "GLES3-only code links to OpenGL!")
  endif()
  if(iface_libs MATCHES "EGL")
    message(FATAL_ERROR "GLES3-GLX-only code links to EGL!")
  endif()
endif()
