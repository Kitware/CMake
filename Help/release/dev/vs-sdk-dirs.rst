vs-sdk-dirs
-----------

* ``CMAKE_VS_SDK_*_DIRECTORIES`` variables were defined to tell
  :ref:`Visual Studio Generators` for VS 2010 and above how to populate
  fields in ``.vcxproj`` files that specify SDK directories.  The
  variables are:

  - :variable:`CMAKE_VS_SDK_EXCLUDE_DIRECTORIES`
  - :variable:`CMAKE_VS_SDK_EXECUTABLE_DIRECTORIES`
  - :variable:`CMAKE_VS_SDK_INCLUDE_DIRECTORIES`
  - :variable:`CMAKE_VS_SDK_LIBRARY_DIRECTORIES`
  - :variable:`CMAKE_VS_SDK_LIBRARY_WINRT_DIRECTORIES`
  - :variable:`CMAKE_VS_SDK_REFERENCE_DIRECTORIES`
  - :variable:`CMAKE_VS_SDK_SOURCE_DIRECTORIES`
