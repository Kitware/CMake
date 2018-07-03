cpack-external
--------------

* CPack gained a new :cpack_gen:`CPack External Generator` which is used to
  export the CPack metadata in a format that other software can understand. The
  intention of this generator is to allow external packaging software to take
  advantage of CPack's features when it may not be possible to use CPack for
  the entire packaging process.
