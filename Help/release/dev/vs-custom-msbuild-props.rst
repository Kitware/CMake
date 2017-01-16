vs-custom-msbuild-props
-----------------------

* The :ref:`Visual Studio Generators` for VS 2010 and above can
  now be fine tuned using custom msbuild .props files.
  :prop_tgt:`VS_USER_PROPS` can be
  used to change the default path of the user .props file from
  ``$(UserRootDir)\\Microsoft.Cpp.$(Platform).user.props`` to
  an arbitrary filename.
