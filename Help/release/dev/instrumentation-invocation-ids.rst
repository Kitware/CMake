instrumentation-invocation-ids
------------------------------

* :manual:`cmake-instrumentation(7)` gained data version ``1.3``, which adds
  ``buildID``, ``ctestID``, and ``installID`` snippet fields to associate
  compile, link, custom, test, and install commands with their parent build,
  CTest, or install invocation.
