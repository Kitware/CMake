set(listvar a b c d e)

list(SUBLIST listvar 0 invalid out)
message("SUBLIST-length: -->${out}<--")
