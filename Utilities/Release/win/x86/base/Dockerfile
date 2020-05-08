# escape=`

# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

# Produce a base image with a build environment for portable CMake binaries.
# Build using the directory containing this file as its own build context.

ARG FROM_IMAGE_NAME=mcr.microsoft.com/dotnet/framework/sdk:4.8-windowsservercore-ltsc2019
ARG FROM_IMAGE_DIGEST=@sha256:a94289bfd61ba89cd162f7cf84afe0e307d4d2576b44b9bd277e7b3036ccfa6b
ARG FROM_IMAGE=$FROM_IMAGE_NAME$FROM_IMAGE_DIGEST
FROM $FROM_IMAGE

# Use a traditional Windows shell.
SHELL ["cmd", "/S", "/C"]

# Install Visual Studio Build Tools for desktop development with C++.
ADD https://aka.ms/vs/16/release/vs_buildtools.exe C:\TEMP\vs_buildtools.exe
RUN C:\TEMP\vs_buildtools.exe --quiet --wait --norestart --nocache `
    --installPath C:\BuildTools `
    --add Microsoft.VisualStudio.Workload.VCTools `
    --add Microsoft.VisualStudio.Component.VC.Tools.x86.x64 `
    --add Microsoft.VisualStudio.Component.VC.CLI.Support `
    --add Microsoft.VisualStudio.Component.VC.ATL `
    --add Microsoft.VisualStudio.Component.Windows10SDK.18362 `
 || IF "%ERRORLEVEL%"=="3010" EXIT 0
RUN del C:\TEMP\vs_buildtools.exe

# Add a toolchain environment loader for each architecture.
COPY msvc-x86_64.bat msvc-i386.bat C:\
