FindProtobuf
------------

* The :module:`FindProtobuf` module's :command:`protobuf_generate(DEPENDENCIES)`
  command argument now accepts multiple values.
* The :module:`FindProtobuf` module's commands :command:`protobuf_generate_cpp`
  and :command:`protobuf_generate_python` together with their hint variables
  ``Protobuf_IMPORT_DIRS`` and ``PROTOBUF_GENERATE_CPP_APPEND_PATH`` are now
  deprecated in favor of :command:`protobuf_generate`.
