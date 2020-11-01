@rem Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
@rem file Copyright.txt or https://cmake.org/licensing for details.

call \msvc.bat && @echo on || exit /b
set PATH=C:\wix;C:\ninja;%PATH%
cd \cmake\src\cmake-build && (
  for %%p in (%*) do (
    bin\cpack -G %%p
  )
) && ^
mkdir \out && ^
move cmake-*-win* \out
