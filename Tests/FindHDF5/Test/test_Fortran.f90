program hdf5_hello
  use hdf5
  integer error
  call h5open_f(error)
  call h5close_f(error)
end
