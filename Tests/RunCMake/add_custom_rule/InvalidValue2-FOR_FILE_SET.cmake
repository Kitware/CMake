
macro(fs)
endmacro()

add_custom_rule(rule OUTPUT out COMMAND cmd CONFIGURATOR FOR_FILE_SET fs)
