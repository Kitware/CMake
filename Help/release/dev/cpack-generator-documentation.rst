cpack-generator-documentation
-----------------------------

* The CPack generators have been moved into their own separate section in the
  documentation, rather than having the documentation in their internal
  implementation modules.
* These internal implementation modules are also no longer available to scripts
  that may have been incorrectly including them, because they should never have
  been available in the first place.
