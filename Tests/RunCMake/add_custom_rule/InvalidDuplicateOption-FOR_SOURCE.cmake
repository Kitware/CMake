
function(for_source)
endfunction()

add_custom_rule(rule COMMAND foo OUTPUT out CONFIGURATOR FOR_SOURCE for_source FOR_SOURCE for_source)
