enable_language(CXX)

function (use_autogen target)
  find_package(Qt${with_qt_version} REQUIRED COMPONENTS Core Widgets)
  set(Qt${with_qt_version}Core_VERSION_MAJOR "${Qt${with_qt_version}Core_VERSION_MAJOR}" PARENT_SCOPE)
  set(Qt${with_qt_version}Core_VERSION_MINOR "${Qt${with_qt_version}Core_VERSION_MINOR}" PARENT_SCOPE)
  set_property(TARGET "${target}" PROPERTY AUTOMOC 1)
  set_property(TARGET "${target}" PROPERTY AUTORCC 1)
  set_property(TARGET "${target}" PROPERTY AUTOUIC 1)
endfunction ()

function (wrap_autogen target)
  use_autogen("${target}")
endfunction ()

add_executable(main empty.cpp)
wrap_autogen(main)
