CMAKE_SYSTEM_ENVIRONMENT_ID
---------------------------

* The :envvar:`CMAKE_SYSTEM_ENVIRONMENT_ID` environment variable was added to
  hint to :manual:`cmake(1)` when :cmake-option:`--fresh` may be necessary,
  along with :envvar:`CMAKE_SYSTEM_ENVIRONMENT_ACTION` to control what happens
  when :envvar:`CMAKE_SYSTEM_ENVIRONMENT_ID` changes.
