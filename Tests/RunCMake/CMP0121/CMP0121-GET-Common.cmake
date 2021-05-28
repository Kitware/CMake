set(listvar a b c d e)

list(GET listvar 0 2junk out)
message("GET: -->${out}<--")
