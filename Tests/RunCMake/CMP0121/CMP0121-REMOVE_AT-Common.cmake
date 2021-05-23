set(listvar a b c d e)

list(REMOVE_AT listvar 0 invalid)
message("REMOVE_AT: -->${listvar}<--")
