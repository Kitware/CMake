subroutine use_long()
  use, non_intrinsic :: split_token_module_with_a_long_name_that_flang_wraps_in_output
  print *, long_value
end subroutine
