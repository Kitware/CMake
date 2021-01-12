cuda-nvcc-ccache-symlink
------------------------

* ``CUDA`` language support now works when ``nvcc`` is a symbolic link,
  for example due to a ``ccache`` or ``colornvcc`` wrapper script.

* The :module:`FindCUDAToolkit` module gained support for finding CUDA
  toolkits when ``nvcc`` is a symbolic link,
  for example due to a ``ccache`` or ``colornvcc`` wrapper script.
