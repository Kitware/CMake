Objective C/C++
---------------

* CMake learned to support the Objective C (``OBJC``) and Objective C++
  (``OBJCXX``) languages.  They may be enabled via the :command:`project`
  and :command:`enable_language` commands.  When ``OBJC`` or ``OBJCXX``
  is enabled, source files with the ``.m`` or ``.mm``, respectively,
  will be compiled as Objective C or C++.  Otherwise they will be treated
  as plain C++ sources as they were before.
