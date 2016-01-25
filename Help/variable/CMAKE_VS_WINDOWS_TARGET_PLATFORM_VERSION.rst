CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION
----------------------------------------

Visual Studio Windows Target Platform Version.

When targeting Windows 10 and above Visual Studio 2015 and above support
specification of a target Windows version to select a corresponding SDK.
The :variable:`CMAKE_SYSTEM_VERSION` variable may be set to specify a
version.  Otherwise CMake computes a default version based on the Windows
SDK versions available.  The chosen Windows target version number is provided
in ``CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION``.  If no Windows 10 SDK
is available this value will be empty.
