FindX11-imported-targets
------------------------

* The :module:`FindX11` had the following variables renamed in order to match
  their library names rather than header names. The old variables are provided
  for compatibility:

    - ``X11_Xxf86misc_INCLUDE_PATH`` instead of ``X11_xf86misc_INCLUDE_PATH``
    - ``X11_Xxf86misc_LIB`` instead of ``X11_xf86misc_LIB``
    - ``X11_Xxf86misc_FOUND`` instead of ``X11_xf86misc_FOUND``
    - ``X11_Xxf86vm_INCLUDE_PATH`` instead of ``X11_xf86vmode_INCLUDE_PATH``
    - ``X11_Xxf86vm_LIB`` instead of ``X11_xf86vmode_LIB``
    - ``X11_Xxf86vm_FOUND`` instead of ``X11_xf86vmode_FOUND``
    - ``X11_xkbfile_INCLUDE_PATH`` instead of ``X11_Xkbfile_INCLUDE_PATH``
    - ``X11_xkbfile_LIB`` instead of ``X11_Xkbfile_LIB``
    - ``X11_xkbfile_FOUND`` instead of ``X11_Xkbfile_FOUND``
    - ``X11_Xtst_INCLUDE_PATH`` instead of ``X11_XTest_INCLUDE_PATH``
    - ``X11_Xtst_LIB`` instead of ``X11_XTest_LIB``
    - ``X11_Xtst_FOUND`` instead of ``X11_XTest_FOUND``
    - ``X11_Xss_INCLUDE_PATH`` instead of ``X11_Xscreensaver_INCLUDE_PATH``
    - ``X11_Xss_LIB`` instead of ``X11_Xscreensaver_LIB``
    - ``X11_Xss_FOUND`` instead of ``X11_Xscreensaver_FOUND``

  The following variables are deprecated completely since they were
  essentially duplicates:

    - ``X11_Xinput_INCLUDE_PATH`` (use ``X11_Xi_INCLUDE_PATH``)
    - ``X11_Xinput_LIB`` (use ``X11_Xi_LIB``)
    - ``X11_Xinput_FOUND`` (use ``X11_Xi_FOUND``)

* The :module:`FindX11` now provides ``X11_Xext_INCLUDE_PATH``.
* The :module:`FindX11` now provides imported targets.
