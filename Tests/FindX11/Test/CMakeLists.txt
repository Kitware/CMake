cmake_minimum_required(VERSION 3.10)
project(TestFindX11 C)
include(CTest)

find_package(X11 REQUIRED)

function (test_x11_component have_var component)
  if (NOT X11_${component}_FOUND)
    message("Skipping ${component} because it was not found.")
    return ()
  endif ()

  add_executable(test_tgt_${component} main.c)
  target_link_libraries(test_tgt_${component} PRIVATE X11::${component})
  target_compile_definitions(test_tgt_${component} PRIVATE HAVE_X11_${component})
  add_test(NAME test_tgt_${component} COMMAND test_tgt_${component})

  # Add to the list of components to test for the parent.
  set(${have_var}
    ${${have_var}}
    HAVE_X11_${component}
    PARENT_SCOPE)
endfunction ()

set(x11_components)
test_x11_component(x11_components ICE)
test_x11_component(x11_components SM)
# Not a component; hack it up.
set(X11_X11_FOUND ${X11_FOUND})
test_x11_component(x11_components X11)
test_x11_component(x11_components Xau)
test_x11_component(x11_components Xaw)
test_x11_component(x11_components xcb)
test_x11_component(x11_components X11_xcb)
test_x11_component(x11_components xcb_icccm)
test_x11_component(x11_components xcb_randr)
test_x11_component(x11_components xcb_util)
test_x11_component(x11_components xcb_xfixes)
test_x11_component(x11_components xcb_xtest)
test_x11_component(x11_components xcb_keysyms)
test_x11_component(x11_components xcb_xkb)
test_x11_component(x11_components Xcomposite)
test_x11_component(x11_components Xdamage)
test_x11_component(x11_components Xdmcp)
test_x11_component(x11_components Xext)
test_x11_component(x11_components Xxf86misc)
test_x11_component(x11_components Xxf86vm)
test_x11_component(x11_components Xfixes)
# We ignore the Xft component because the variables do not provide the required
# dependency information (Freetype and Fontconfig).
test_x11_component(x11_components_ignore Xft)
test_x11_component(x11_components Xi)
test_x11_component(x11_components Xinerama)
test_x11_component(x11_components xkbcommon)
test_x11_component(x11_components xkbcommon_X11)
test_x11_component(x11_components Xkb)
test_x11_component(x11_components xkbfile)
test_x11_component(x11_components Xmu)
test_x11_component(x11_components Xpm)
test_x11_component(x11_components Xtst)
test_x11_component(x11_components Xrandr)
test_x11_component(x11_components Xrender)
test_x11_component(x11_components XRes)
test_x11_component(x11_components Xss)
test_x11_component(x11_components Xt)
test_x11_component(x11_components Xutil)
test_x11_component(x11_components Xv)

# The variables do not include dependency information. Just test "everything".
add_executable(test_var main.c)
target_include_directories(test_var PRIVATE ${X11_INCLUDE_DIRS})
target_link_libraries(test_var PRIVATE ${X11_LIBRARIES})
# Not included in X11_LIBRARIES.
foreach(lib
    Xau
    Xaw
    xcb
    X11_xcb
    xcb_icccm
    xcb_randr
    xcb_util
    xcb_xfixes
    Xcomposite
    Xdamage
    Xdmcp
    Xxf86misc
    Xxf86vm
    Xfixes
    Xi
    Xinerama
    xkbcommon
    xkbcommon_X11
    Xkb
    xkbfile
    Xmu
    Xpm
    Xtst
    Xrandr
    Xrender
    XRes
    Xss
    Xt
    Xv
    )
  if (X11_${lib}_FOUND)
    target_link_libraries(test_var PRIVATE ${X11_${lib}_LIB})
  endif ()
endforeach()
target_compile_definitions(test_var PRIVATE ${x11_components})
add_test(NAME test_var COMMAND test_var)
