directory-list-targets-and-subdirs
----------------------------------

* A :prop_dir:`SOURCE_DIR` directory property was added to get the
  absolute path to the source directory associated with a directory.

* A :prop_dir:`BINARY_DIR` directory property was added to get the
  absolute path to the binary directory corresponding to the source
  directory on which the property is read.

* A :prop_dir:`BUILDSYSTEM_TARGETS` directory property was added to
  get the list of logical buildsystem target names added by the
  project in a directory.

* A :prop_dir:`SUBDIRECTORIES` directory property was added to
  get the list of subdirectories added by a project in a directory.
