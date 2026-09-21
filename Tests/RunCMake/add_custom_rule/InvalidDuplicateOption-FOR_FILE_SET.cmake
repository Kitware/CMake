
function(for_fs)
endfunction()

add_custom_rule(rule COMMAND foo OUTPUT out CONFIGURATOR FOR_FILE_SET for_fs FOR_FILE_SET for_fs)
