cpack-rpm-omit-unsupported-tags
-------------------------------

* The :module:`CPackRPM` module learned to omit
  tags that are not supported by provided
  rpmbuild tool. If unsupported tags are set
  they are ignored and a developer warning is
  printed out.
