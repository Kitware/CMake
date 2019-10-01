UNITY_BUILD_BATCH_SIZE
----------------------

Specifies how many source code files will be included into a
:prop_tgt:`UNITY_BUILD` source file.

If the property is not set, CMake will use the value provided
by :variable:`CMAKE_UNITY_BUILD_BATCH_SIZE`.

By setting it to value `0` the generated unity source file will
contain all the source files that would otherwise be split
into multiple batches. It is not recommended to do so, since it
would affect performance.
