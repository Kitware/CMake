vs-special-source-file-properties
---------------------------------

* A :prop_sf:`VS_DEPLOYMENT_CONTENT` source file property was added
  to tell the Visual Studio generators to mark content for deployment
  in Windows Phone and Windows Store projects.

* The Visual Studio generators learned to treat ``.hlsl`` source
  files as High Level Shading Language sources (using ``FXCompile``
  in ``.vcxproj`` files).  A :prop_sf:`VS_SHADER_TYPE` source file
  property was added to specify the Shader Type.
