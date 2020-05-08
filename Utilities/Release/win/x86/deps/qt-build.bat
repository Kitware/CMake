set ARCH=%1
call \msvc-%ARCH%.bat && @echo on || exit /b
mkdir \qt-src\qt-build && ^
cd \qt-src\qt-build && ^
..\qt\configure.bat ^
  -prefix C:/qt-%ARCH% ^
  -static ^
  -static-runtime ^
  -release ^
  -opensource -confirm-license ^
  -platform win32-msvc ^
  -mp ^
  -gui ^
  -widgets ^
  -qt-pcre ^
  -qt-zlib ^
  -qt-libpng ^
  -qt-libjpeg ^
  -no-gif ^
  -no-icu ^
  -no-pch ^
  -no-angle ^
  -no-opengl ^
  -no-dbus ^
  -no-harfbuzz ^
  -no-accessibility ^
  -skip declarative ^
  -skip multimedia ^
  -skip qtcanvas3d ^
  -skip qtconnectivity ^
  -skip qtdeclarative ^
  -skip qtlocation ^
  -skip qtmultimedia ^
  -skip qtsensors ^
  -skip qtserialport ^
  -skip qtsvg ^
  -skip qtwayland ^
  -skip qtwebchannel ^
  -skip qtwebengine ^
  -skip qtwebsockets ^
  -skip qtxmlpatterns ^
  -nomake examples -nomake tests ^
  && ^
\jom\jom.exe -J %NUMBER_OF_PROCESSORS% && ^
\jom\jom.exe install && ^
cd \qt-%ARCH% && ^
\git\cmd\git apply \qt-src\qt-install.patch
