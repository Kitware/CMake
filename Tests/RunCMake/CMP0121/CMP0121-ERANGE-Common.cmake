set(listvar a b c d e)

list(GET listvar
  18446744073709551616 # 2^64
  2147483648 # 2^31
  4294967296 # 2^32; errors out-of-range as -2147483643 due to underflow
  out)
message("ERANGE: -->${out}<--")
