enable_language(CXX)
add_library(foo foo.cpp)

set(test1Import "path/to/nuget_packages/Foo.1.0.0/build/Foo.props")
set(test2Import "path/to/nuget_packages/Bar.1.0.0/build/Bar.props")

set_property(TARGET foo PROPERTY
  VS_PROJECT_IMPORT
    ${test1Import}
    ${test2Import}
  )
