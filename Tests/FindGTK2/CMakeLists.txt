find_package(GTK2 COMPONENTS gtk glade gtkmm glademm QUIET)

if(GTK2_GLIB_FOUND)
  add_test(GTK2Targets.glib ${CMAKE_CTEST_COMMAND}
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/FindGTK2/glib"
    "${CMake_BINARY_DIR}/Tests/FindGTK2/glib"
    ${build_generator_args}
    --build-project glib
    --build-exe-dir "${CMake_BINARY_DIR}/Tests/FindGTK2/glib"
    --force-new-ctest-process
    --test-command ${CMAKE_CTEST_COMMAND} -V
    )
endif()

if(GTK2_GOBJECT_FOUND)
  add_test(GTK2Targets.gobject ${CMAKE_CTEST_COMMAND}
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/FindGTK2/gobject"
    "${CMake_BINARY_DIR}/Tests/FindGTK2/gobject"
    ${build_generator_args}
    --build-project gobject
    --build-exe-dir "${CMake_BINARY_DIR}/Tests/FindGTK2/gobject"
    --force-new-ctest-process
    --test-command ${CMAKE_CTEST_COMMAND} -V
    )
endif()

if(GTK2_GIO_FOUND)
  add_test(GTK2Targets.gio ${CMAKE_CTEST_COMMAND}
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/FindGTK2/gio"
    "${CMake_BINARY_DIR}/Tests/FindGTK2/gio"
    ${build_generator_args}
    --build-project gio
    --build-exe-dir "${CMake_BINARY_DIR}/Tests/FindGTK2/gio"
    --force-new-ctest-process
    --test-command ${CMAKE_CTEST_COMMAND} -V
    )
endif()

if(GTK2_GMODULE_FOUND)
  add_test(GTK2Targets.gmodule ${CMAKE_CTEST_COMMAND}
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/FindGTK2/gmodule"
    "${CMake_BINARY_DIR}/Tests/FindGTK2/gmodule"
    ${build_generator_args}
    --build-project gmodule
    --build-exe-dir "${CMake_BINARY_DIR}/Tests/FindGTK2/gmodule"
    --force-new-ctest-process
    --test-command ${CMAKE_CTEST_COMMAND} -V
    )
endif()

if(GTK2_GTHREAD_FOUND)
  add_test(GTK2Targets.gthread ${CMAKE_CTEST_COMMAND}
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/FindGTK2/gthread"
    "${CMake_BINARY_DIR}/Tests/FindGTK2/gthread"
    ${build_generator_args}
    --build-project gthread
    --build-exe-dir "${CMake_BINARY_DIR}/Tests/FindGTK2/gthread"
    --force-new-ctest-process
    --test-command ${CMAKE_CTEST_COMMAND} -V
    )
endif()

if(GTK2_ATK_FOUND)
  add_test(GTK2Targets.atk ${CMAKE_CTEST_COMMAND}
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/FindGTK2/atk"
    "${CMake_BINARY_DIR}/Tests/FindGTK2/atk"
    ${build_generator_args}
    --build-project atk
    --build-exe-dir "${CMake_BINARY_DIR}/Tests/FindGTK2/atk"
    --force-new-ctest-process
    --test-command ${CMAKE_CTEST_COMMAND} -V
    )
endif()

if(GTK2_GDK_PIXBUF_FOUND)
  add_test(GTK2Targets.gdk_pixbuf ${CMAKE_CTEST_COMMAND}
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/FindGTK2/gdk_pixbuf"
    "${CMake_BINARY_DIR}/Tests/FindGTK2/gdk_pixbuf"
    ${build_generator_args}
    --build-project gdk_pixbuf
    --build-exe-dir "${CMake_BINARY_DIR}/Tests/FindGTK2/gdk_pixbuf"
    --force-new-ctest-process
    --test-command ${CMAKE_CTEST_COMMAND} -V
    )
endif()

if(GTK2_CAIRO_FOUND)
  add_test(GTK2Targets.cairo ${CMAKE_CTEST_COMMAND}
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/FindGTK2/cairo"
    "${CMake_BINARY_DIR}/Tests/FindGTK2/cairo"
    ${build_generator_args}
    --build-project cairo
    --build-exe-dir "${CMake_BINARY_DIR}/Tests/FindGTK2/cairo"
    --force-new-ctest-process
    --test-command ${CMAKE_CTEST_COMMAND} -V
    )
endif()

if(GTK2_PANGO_FOUND)
  add_test(GTK2Targets.pango ${CMAKE_CTEST_COMMAND}
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/FindGTK2/pango"
    "${CMake_BINARY_DIR}/Tests/FindGTK2/pango"
    ${build_generator_args}
    --build-project pango
    --build-exe-dir "${CMake_BINARY_DIR}/Tests/FindGTK2/pango"
    --force-new-ctest-process
    --test-command ${CMAKE_CTEST_COMMAND} -V
    )
endif()

if(GTK2_PANGOCAIRO_FOUND)
  add_test(GTK2Targets.pangocairo ${CMAKE_CTEST_COMMAND}
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/FindGTK2/pangocairo"
    "${CMake_BINARY_DIR}/Tests/FindGTK2/pangocairo"
    ${build_generator_args}
    --build-project pangocairo
    --build-exe-dir "${CMake_BINARY_DIR}/Tests/FindGTK2/pangocairo"
    --force-new-ctest-process
    --test-command ${CMAKE_CTEST_COMMAND} -V
    )
endif()

if(GTK2_PANGOXFT_FOUND)
  add_test(GTK2Targets.pangoxft ${CMAKE_CTEST_COMMAND}
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/FindGTK2/pangoxft"
    "${CMake_BINARY_DIR}/Tests/FindGTK2/pangoxft"
    ${build_generator_args}
    --build-project pangoxft
    --build-exe-dir "${CMake_BINARY_DIR}/Tests/FindGTK2/pangoxft"
    --force-new-ctest-process
    --test-command ${CMAKE_CTEST_COMMAND} -V
    )
endif()

if(GTK2_PANGOFT2_FOUND)
  add_test(GTK2Targets.pangoft2 ${CMAKE_CTEST_COMMAND}
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/FindGTK2/pangoft2"
    "${CMake_BINARY_DIR}/Tests/FindGTK2/pangoft2"
    ${build_generator_args}
    --build-project pangoft2
    --build-exe-dir "${CMake_BINARY_DIR}/Tests/FindGTK2/pangoft2"
    --force-new-ctest-process
    --test-command ${CMAKE_CTEST_COMMAND} -V
    )
endif()

if(GTK2_GDK_FOUND)
  add_test(GTK2Targets.gdk ${CMAKE_CTEST_COMMAND}
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/FindGTK2/gdk"
    "${CMake_BINARY_DIR}/Tests/FindGTK2/gdk"
    ${build_generator_args}
    --build-project gdk
    --build-exe-dir "${CMake_BINARY_DIR}/Tests/FindGTK2/gdk"
    --force-new-ctest-process
    --test-command ${CMAKE_CTEST_COMMAND} -V
    )
endif()

if(GTK2_GTK_FOUND)
  add_test(GTK2Targets.gtk ${CMAKE_CTEST_COMMAND}
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/FindGTK2/gtk"
    "${CMake_BINARY_DIR}/Tests/FindGTK2/gtk"
    ${build_generator_args}
    --build-project gtk
    --build-exe-dir "${CMake_BINARY_DIR}/Tests/FindGTK2/gtk"
    --force-new-ctest-process
    --test-command ${CMAKE_CTEST_COMMAND} -V
    )

  add_test(GTK2Components.gtk ${CMAKE_CTEST_COMMAND}
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/FindGTK2/gtk"
    "${CMake_BINARY_DIR}/Tests/FindGTK2/gtk"
     ${build_generator_args}
    --build-target gtk-all-libs
    --build-exe-dir "${CMake_BINARY_DIR}/Tests/FindGTK2/gtk"
    --force-new-ctest-process
    --test-command ${CMAKE_CTEST_COMMAND} -V
    )
endif()

if(GTK2_SIGC++_FOUND)
  add_test(GTK2Targets.sigc++ ${CMAKE_CTEST_COMMAND}
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/FindGTK2/sigc++"
    "${CMake_BINARY_DIR}/Tests/FindGTK2/sigc++"
     ${build_generator_args}
    --build-project sigc++
    --build-exe-dir "${CMake_BINARY_DIR}/Tests/FindGTK2/sigc++"
    --force-new-ctest-process
    --test-command ${CMAKE_CTEST_COMMAND} -V
    )
endif()

if(GTK2_GLIBMM_FOUND)
  add_test(GTK2Targets.glibmm ${CMAKE_CTEST_COMMAND}
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/FindGTK2/glibmm"
    "${CMake_BINARY_DIR}/Tests/FindGTK2/glibmm"
     ${build_generator_args}
    --build-project glibmm
    --build-exe-dir "${CMake_BINARY_DIR}/Tests/FindGTK2/glibmm"
    --force-new-ctest-process
    --test-command ${CMAKE_CTEST_COMMAND} -V
    )
endif()

if(GTK2_GIOMM_FOUND)
  add_test(GTK2Targets.giomm ${CMAKE_CTEST_COMMAND}
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/FindGTK2/giomm"
    "${CMake_BINARY_DIR}/Tests/FindGTK2/giomm"
     ${build_generator_args}
    --build-project giomm
    --build-exe-dir "${CMake_BINARY_DIR}/Tests/FindGTK2/giomm"
    --force-new-ctest-process
    --test-command ${CMAKE_CTEST_COMMAND} -V
    )
endif()

if(GTK2_ATKMM_FOUND)
  add_test(GTK2Targets.atkmm ${CMAKE_CTEST_COMMAND}
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/FindGTK2/atkmm"
    "${CMake_BINARY_DIR}/Tests/FindGTK2/atkmm"
     ${build_generator_args}
    --build-project atkmm
    --build-exe-dir "${CMake_BINARY_DIR}/Tests/FindGTK2/atkmm"
    --force-new-ctest-process
    --test-command ${CMAKE_CTEST_COMMAND} -V
    )
endif()

if(GTK2_CAIROMM_FOUND)
  add_test(GTK2Targets.cairomm ${CMAKE_CTEST_COMMAND}
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/FindGTK2/cairomm"
    "${CMake_BINARY_DIR}/Tests/FindGTK2/cairomm"
     ${build_generator_args}
    --build-project cairomm
    --build-exe-dir "${CMake_BINARY_DIR}/Tests/FindGTK2/cairomm"
    --force-new-ctest-process
    --test-command ${CMAKE_CTEST_COMMAND} -V
    )
endif()

if(GTK2_PANGOMM_FOUND)
  add_test(GTK2Targets.pangomm ${CMAKE_CTEST_COMMAND}
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/FindGTK2/pangomm"
    "${CMake_BINARY_DIR}/Tests/FindGTK2/pangomm"
     ${build_generator_args}
    --build-project pangomm
    --build-exe-dir "${CMake_BINARY_DIR}/Tests/FindGTK2/pangomm"
    --force-new-ctest-process
    --test-command ${CMAKE_CTEST_COMMAND} -V
    )
endif()

if(GTK2_GDKMM_FOUND)
  add_test(GTK2Targets.gdkmm ${CMAKE_CTEST_COMMAND}
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/FindGTK2/gdkmm"
    "${CMake_BINARY_DIR}/Tests/FindGTK2/gdkmm"
     ${build_generator_args}
    --build-project gdkmm
    --build-exe-dir "${CMake_BINARY_DIR}/Tests/FindGTK2/gdkmm"
    --force-new-ctest-process
    --test-command ${CMAKE_CTEST_COMMAND} -V
    )
endif()

if(GTK2_GTKMM_FOUND)
  add_test(GTK2Targets.gtkmm ${CMAKE_CTEST_COMMAND}
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/FindGTK2/gtkmm"
    "${CMake_BINARY_DIR}/Tests/FindGTK2/gtkmm"
     ${build_generator_args}
    --build-target gtkmm-target
    --build-exe-dir "${CMake_BINARY_DIR}/Tests/FindGTK2/gtkmm"
    --force-new-ctest-process
    --test-command ${CMAKE_CTEST_COMMAND} -V
    )

  add_test(GTK2Components.gtkmm ${CMAKE_CTEST_COMMAND}
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/FindGTK2/gtkmm"
    "${CMake_BINARY_DIR}/Tests/FindGTK2/gtkmm"
     ${build_generator_args}
    --build-target gtkmm-all-libs
    --build-exe-dir "${CMake_BINARY_DIR}/Tests/FindGTK2/gtkmm"
    --force-new-ctest-process
    --test-command ${CMAKE_CTEST_COMMAND} -V
    )
endif()
