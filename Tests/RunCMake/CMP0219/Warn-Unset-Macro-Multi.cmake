set(cmp0219_warn_value1 "prefix\\tsuffix")
set(cmp0219_warn_value2 "left\\;right")

macro(cmp0219_warn_macro arg1 arg2)
  set(cmp0219_warn_seen1 "${arg1}")
  set(cmp0219_warn_seen2 "${arg2}")
endmacro()

cmp0219_warn_macro("${cmp0219_warn_value1}" "${cmp0219_warn_value2}")
