@rem Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
@rem file Copyright.txt or https://cmake.org/licensing for details.

set ARCH=%1
set TEST=%2

copy \msvc-%ARCH%.bat \msvc.bat
call \msvc.bat && @echo on || exit /b
set PATH=C:\ninja;%PATH%

mkdir \cmake\src\cmake-build && ^
cd \cmake\src\cmake-build && ^
copy ..\cmake\Utilities\Release\win\x86\cache-%ARCH%.txt CMakeCache.txt && ^
\cmake\cmake\bin\cmake ..\cmake -GNinja && ^
ninja && (
  if "%TEST%"=="true" (
    bin\ctest --output-on-failure -j %NUMBER_OF_PROCESSORS% -R "^(CMake\.|CMakeLib\.|CMakeServerLib\.|RunCMake\.ctest_memcheck)"
  )
)
