ANDROID_STL_TYPE
----------------

Set the Android property that defines the type of STL support for the project.
This is a string property that could set to the one of the following values:
``none``           e.g. "No C++ Support"
``system``         e.g. "Minimal C++ without STL"
``gabi++_static``  e.g. "GAbi++ Static"
``gabi++_shared``  e.g. "GAbi++ Shared"
``gnustl_static``  e.g. "GNU libstdc++ Static"
``gnustl_shared``  e.g. "GNU libstdc++ Shared"
``stlport_static`` e.g. "STLport Static"
``stlport_shared`` e.g. "STLport Shared"
This property is initialized by the value of the
variable:`CMAKE_ANDROID_STL_TYPE` variable if it is set when a target is created.
