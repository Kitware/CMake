set-env-var-first-run
---------------------

* CMake no longer sets environment variables like :envvar:`CC`, :envvar:`CXX`,
  etc. when enabling the corresponding language during the first CMake run in
  a build directory. See policy :policy:`CMP0132`.
