deprecate-policy-old
--------------------

* Compatibility with versions of CMake older than 3.5 is now deprecated
  and will be removed from a future version.  Calls to
  :command:`cmake_minimum_required` or :command:`cmake_policy` that set
  the policy version to an older value now issue a deprecation diagnostic.
