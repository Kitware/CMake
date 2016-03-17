FindProtobuf-variable-case
--------------------------

* The :module:`FindProtobuf` module input and output variables were all renamed
  from ``PROTOBUF_`` to ``Protobuf_`` for consistency with other find modules.
  Input variables of the old case will be honored if provided, and output
  variables of the old case are always provided.
