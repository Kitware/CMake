@rem Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
@rem file Copyright.txt or https://cmake.org/licensing for details.

set ARCH=%1
call \msvc-%ARCH%.bat && @echo on || exit /b
set "PATH=C:\cmake\cmake\bin;C:\python3;%PATH%"
mkdir \cmake\src\cmake-nmake && ^
cd \cmake\src\cmake-nmake && ^
> CMakeCache.txt (
  @echo CMAKE_Fortran_COMPILER:STRING=
  @echo CMAKE_Swift_COMPILER:STRING=
  @echo CMake_TEST_IPO_WORKS_C:BOOL=ON
  @echo CMake_TEST_IPO_WORKS_CXX:BOOL=ON
  @echo CMake_TEST_NO_NETWORK:BOOL=ON
  @echo CTEST_RUN_MFC:BOOL=OFF
) && ^
cmake ..\cmake -DCMake_TEST_HOST_CMAKE=1 -G "NMake Makefiles" && ^
nmake && ^
ctest --output-on-failure -j %NUMBER_OF_PROCESSORS%
