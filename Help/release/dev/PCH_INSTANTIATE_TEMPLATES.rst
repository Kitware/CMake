PCH_INSTANTIATE_TEMPLATES
-------------------------

* The :prop_tgt:`PCH_INSTANTIATE_TEMPLATES` target property was added to enable
  template instantiation in the precompiled header. This is enabled by default
  and offers a roughly 20% compile time improvement. Currently only supported
  by Clang 11.
