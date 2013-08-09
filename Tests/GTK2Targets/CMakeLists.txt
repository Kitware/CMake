find_package(GTK2 COMPONENTS gtk glade gtkmm glademm QUIET)

if(GTK2_GLIB_FOUND)
  add_test(GTK2Targets.glib ${CMAKE_CTEST_COMMAND}
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/GTK2Targets/glib"
    "${CMake_BINARY_DIR}/Tests/GTK2Targets/glib"
    ${build_generator_args}
    --build-project glib
    --build-exe-dir "${CMake_BINARY_DIR}/Tests/GTK2Targets/glib"
    --force-new-ctest-process
    --test-command ${CMAKE_CTEST_COMMAND} -V
    )
endif()

if(GTK2_GOBJECT_FOUND)
  add_test(GTK2Targets.gobject ${CMAKE_CTEST_COMMAND}
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/GTK2Targets/gobject"
    "${CMake_BINARY_DIR}/Tests/GTK2Targets/gobject"
    ${build_generator_args}
    --build-project gobject
    --build-exe-dir "${CMake_BINARY_DIR}/Tests/GTK2Targets/gobject"
    --force-new-ctest-process
    --test-command ${CMAKE_CTEST_COMMAND} -V
    )
endif()

if(GTK2_GIO_FOUND)
  add_test(GTK2Targets.gio ${CMAKE_CTEST_COMMAND}
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/GTK2Targets/gio"
    "${CMake_BINARY_DIR}/Tests/GTK2Targets/gio"
    ${build_generator_args}
    --build-project gio
    --build-exe-dir "${CMake_BINARY_DIR}/Tests/GTK2Targets/gio"
    --force-new-ctest-process
    --test-command ${CMAKE_CTEST_COMMAND} -V
    )
endif()

if(GTK2_GMODULE_FOUND)
  add_test(GTK2Targets.gmodule ${CMAKE_CTEST_COMMAND}
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/GTK2Targets/gmodule"
    "${CMake_BINARY_DIR}/Tests/GTK2Targets/gmodule"
    ${build_generator_args}
    --build-project gmodule
    --build-exe-dir "${CMake_BINARY_DIR}/Tests/GTK2Targets/gmodule"
    --force-new-ctest-process
    --test-command ${CMAKE_CTEST_COMMAND} -V
    )
endif()

if(GTK2_GTHREAD_FOUND)
  add_test(GTK2Targets.gthread ${CMAKE_CTEST_COMMAND}
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/GTK2Targets/gthread"
    "${CMake_BINARY_DIR}/Tests/GTK2Targets/gthread"
    ${build_generator_args}
    --build-project gthread
    --build-exe-dir "${CMake_BINARY_DIR}/Tests/GTK2Targets/gthread"
    --force-new-ctest-process
    --test-command ${CMAKE_CTEST_COMMAND} -V
    )
endif()

if(GTK2_ATK_FOUND)
  add_test(GTK2Targets.atk ${CMAKE_CTEST_COMMAND}
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/GTK2Targets/atk"
    "${CMake_BINARY_DIR}/Tests/GTK2Targets/atk"
    ${build_generator_args}
    --build-project atk
    --build-exe-dir "${CMake_BINARY_DIR}/Tests/GTK2Targets/atk"
    --force-new-ctest-process
    --test-command ${CMAKE_CTEST_COMMAND} -V
    )
endif()

if(GTK2_GDK_PIXBUF_FOUND)
  add_test(GTK2Targets.gdk_pixbuf ${CMAKE_CTEST_COMMAND}
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/GTK2Targets/gdk_pixbuf"
    "${CMake_BINARY_DIR}/Tests/GTK2Targets/gdk_pixbuf"
    ${build_generator_args}
    --build-project gdk_pixbuf
    --build-exe-dir "${CMake_BINARY_DIR}/Tests/GTK2Targets/gdk_pixbuf"
    --force-new-ctest-process
    --test-command ${CMAKE_CTEST_COMMAND} -V
    )
endif()

if(GTK2_CAIRO_FOUND)
  add_test(GTK2Targets.cairo ${CMAKE_CTEST_COMMAND}
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/GTK2Targets/cairo"
    "${CMake_BINARY_DIR}/Tests/GTK2Targets/cairo"
    ${build_generator_args}
    --build-project cairo
    --build-exe-dir "${CMake_BINARY_DIR}/Tests/GTK2Targets/cairo"
    --force-new-ctest-process
    --test-command ${CMAKE_CTEST_COMMAND} -V
    )
endif()

if(GTK2_PANGO_FOUND)
  add_test(GTK2Targets.pango ${CMAKE_CTEST_COMMAND}
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/GTK2Targets/pango"
    "${CMake_BINARY_DIR}/Tests/GTK2Targets/pango"
    ${build_generator_args}
    --build-project pango
    --build-exe-dir "${CMake_BINARY_DIR}/Tests/GTK2Targets/pango"
    --force-new-ctest-process
    --test-command ${CMAKE_CTEST_COMMAND} -V
    )
endif()

if(GTK2_PANGOCAIRO_FOUND)
  add_test(GTK2Targets.pangocairo ${CMAKE_CTEST_COMMAND}
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/GTK2Targets/pangocairo"
    "${CMake_BINARY_DIR}/Tests/GTK2Targets/pangocairo"
    ${build_generator_args}
    --build-project pangocairo
    --build-exe-dir "${CMake_BINARY_DIR}/Tests/GTK2Targets/pangocairo"
    --force-new-ctest-process
    --test-command ${CMAKE_CTEST_COMMAND} -V
    )
endif()

if(GTK2_PANGOXFT_FOUND)
  add_test(GTK2Targets.pangoxft ${CMAKE_CTEST_COMMAND}
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/GTK2Targets/pangoxft"
    "${CMake_BINARY_DIR}/Tests/GTK2Targets/pangoxft"
    ${build_generator_args}
    --build-project pangoxft
    --build-exe-dir "${CMake_BINARY_DIR}/Tests/GTK2Targets/pangoxft"
    --force-new-ctest-process
    --test-command ${CMAKE_CTEST_COMMAND} -V
    )
endif()

if(GTK2_PANGOFT2_FOUND)
  add_test(GTK2Targets.pangoft2 ${CMAKE_CTEST_COMMAND}
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/GTK2Targets/pangoft2"
    "${CMake_BINARY_DIR}/Tests/GTK2Targets/pangoft2"
    ${build_generator_args}
    --build-project pangoft2
    --build-exe-dir "${CMake_BINARY_DIR}/Tests/GTK2Targets/pangoft2"
    --force-new-ctest-process
    --test-command ${CMAKE_CTEST_COMMAND} -V
    )
endif()

if(GTK2_GDK_FOUND)
  add_test(GTK2Targets.gdk ${CMAKE_CTEST_COMMAND}
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/GTK2Targets/gdk"
    "${CMake_BINARY_DIR}/Tests/GTK2Targets/gdk"
    ${build_generator_args}
    --build-project gdk
    --build-exe-dir "${CMake_BINARY_DIR}/Tests/GTK2Targets/gdk"
    --force-new-ctest-process
    --test-command ${CMAKE_CTEST_COMMAND} -V
    )
endif()

if(GTK2_GTK_FOUND)
  add_test(GTK2Targets.gtk ${CMAKE_CTEST_COMMAND}
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/GTK2Targets/gtk"
    "${CMake_BINARY_DIR}/Tests/GTK2Targets/gtk"
    ${build_generator_args}
    --build-project gtk
    --build-exe-dir "${CMake_BINARY_DIR}/Tests/GTK2Targets/gtk"
    --force-new-ctest-process
    --test-command ${CMAKE_CTEST_COMMAND} -V
    )
endif()

if(GTK2_SIGC++_FOUND)
  add_test(GTK2Targets.sigc++ ${CMAKE_CTEST_COMMAND}
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/GTK2Targets/sigc++"
    "${CMake_BINARY_DIR}/Tests/GTK2Targets/sigc++"
     ${build_generator_args}
    --build-project sigc++
    --build-exe-dir "${CMake_BINARY_DIR}/Tests/GTK2Targets/sigc++"
    --force-new-ctest-process
    --test-command ${CMAKE_CTEST_COMMAND} -V
    )
endif()

if(GTK2_GLIBMM_FOUND)
  add_test(GTK2Targets.glibmm ${CMAKE_CTEST_COMMAND}
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/GTK2Targets/glibmm"
    "${CMake_BINARY_DIR}/Tests/GTK2Targets/glibmm"
     ${build_generator_args}
    --build-project glibmm
    --build-exe-dir "${CMake_BINARY_DIR}/Tests/GTK2Targets/glibmm"
    --force-new-ctest-process
    --test-command ${CMAKE_CTEST_COMMAND} -V
    )
endif()

if(GTK2_GIOMM_FOUND)
  add_test(GTK2Targets.giomm ${CMAKE_CTEST_COMMAND}
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/GTK2Targets/giomm"
    "${CMake_BINARY_DIR}/Tests/GTK2Targets/giomm"
     ${build_generator_args}
    --build-project giomm
    --build-exe-dir "${CMake_BINARY_DIR}/Tests/GTK2Targets/giomm"
    --force-new-ctest-process
    --test-command ${CMAKE_CTEST_COMMAND} -V
    )
endif()

if(GTK2_ATKMM_FOUND)
  add_test(GTK2Targets.atkmm ${CMAKE_CTEST_COMMAND}
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/GTK2Targets/atkmm"
    "${CMake_BINARY_DIR}/Tests/GTK2Targets/atkmm"
     ${build_generator_args}
    --build-project atkmm
    --build-exe-dir "${CMake_BINARY_DIR}/Tests/GTK2Targets/atkmm"
    --force-new-ctest-process
    --test-command ${CMAKE_CTEST_COMMAND} -V
    )
endif()

if(GTK2_CAIROMM_FOUND)
  add_test(GTK2Targets.cairomm ${CMAKE_CTEST_COMMAND}
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/GTK2Targets/cairomm"
    "${CMake_BINARY_DIR}/Tests/GTK2Targets/cairomm"
     ${build_generator_args}
    --build-project cairomm
    --build-exe-dir "${CMake_BINARY_DIR}/Tests/GTK2Targets/cairomm"
    --force-new-ctest-process
    --test-command ${CMAKE_CTEST_COMMAND} -V
    )
endif()

if(GTK2_PANGOMM_FOUND)
  add_test(GTK2Targets.pangomm ${CMAKE_CTEST_COMMAND}
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/GTK2Targets/pangomm"
    "${CMake_BINARY_DIR}/Tests/GTK2Targets/pangomm"
     ${build_generator_args}
    --build-project pangomm
    --build-exe-dir "${CMake_BINARY_DIR}/Tests/GTK2Targets/pangomm"
    --force-new-ctest-process
    --test-command ${CMAKE_CTEST_COMMAND} -V
    )
endif()

if(GTK2_GDKMM_FOUND)
  add_test(GTK2Targets.gdkmm ${CMAKE_CTEST_COMMAND}
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/GTK2Targets/gdkmm"
    "${CMake_BINARY_DIR}/Tests/GTK2Targets/gdkmm"
     ${build_generator_args}
    --build-project gdkmm
    --build-exe-dir "${CMake_BINARY_DIR}/Tests/GTK2Targets/gdkmm"
    --force-new-ctest-process
    --test-command ${CMAKE_CTEST_COMMAND} -V
    )
endif()

if(GTK2_GTKMM_FOUND)
  add_test(GTK2Targets.gtkmm ${CMAKE_CTEST_COMMAND}
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/GTK2Targets/gtkmm"
    "${CMake_BINARY_DIR}/Tests/GTK2Targets/gtkmm"
     ${build_generator_args}
    --build-project gtkmm
    --build-exe-dir "${CMake_BINARY_DIR}/Tests/GTK2Targets/gtkmm"
    --force-new-ctest-process
    --test-command ${CMAKE_CTEST_COMMAND} -V
    )
endif()
