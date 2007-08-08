PROJECT(FindPackageTest)

# Look for a package that has a find module and may be found.
FIND_PACKAGE(OpenGL QUIET)

# Look for a package that has no find module and will not be found.
FIND_PACKAGE(NotAPackage QUIET)

# Look for a package that has an advanced find module.
FIND_PACKAGE(VTK QUIET)

ADD_EXECUTABLE(FindPackageTest FindPackageTest.cxx)
