set(cmp0219_warn_value "prefix\\tsuffix")

macro(cmp0219_warn_macro value)
  set(cmp0219_warn_seen "${value}")
endmacro()

cmp0219_warn_macro("${cmp0219_warn_value}")
cmp0219_warn_macro("${cmp0219_warn_value}")
