FindOpenGL-no-X11
-----------------

* The :module:`FindOpenGL` module no longer explicitly searches
  for any dependency on X11 libraries with the :module:`FindX11`
  module.  Such dependencies should not need to be explicit.
  Applications using X11 APIs themselves should find and link
  to X11 libraries explicitly.
