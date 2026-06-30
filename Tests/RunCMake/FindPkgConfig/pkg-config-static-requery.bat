@ECHO OFF

set static=false
set mode=
set pkg=
set variable=

:LOOP
IF "%~1"=="" GOTO RUN

IF "%~1"=="--version" (
  ECHO 1.0
  EXIT /B 0
)
IF "%~1"=="--static" (
  set static=true
  SHIFT
  GOTO LOOP
)
IF "%~1"=="--modversion" (
  set mode=modversion
  SHIFT
  GOTO LOOP
)
IF "%~1"=="--cflags" (
  set mode=cflags
  SHIFT
  GOTO LOOP
)
IF "%~1"=="--libs" (
  set mode=libs
  SHIFT
  GOTO LOOP
)
IF "%~1"=="--print-errors" (
  SHIFT
  GOTO LOOP
)
IF "%~1"=="--short-errors" (
  SHIFT
  GOTO LOOP
)

set arg=%~1
IF "%arg:~0,11%"=="--variable=" (
  set mode=variable
  set variable=%arg:~11%
) ELSE (
  set pkg=%~1
)
SHIFT
GOTO LOOP

:RUN
IF "%mode%"=="modversion" (
  IF "%pkg%"=="good" GOTO VERSION
  IF "%pkg%"=="bad" GOTO VERSION
)
IF "%mode%"=="variable" (
  IF "%pkg%"=="good" GOTO VARIABLE
  IF "%pkg%"=="bad" GOTO VARIABLE
)
IF "%mode%"=="cflags" (
  IF "%pkg%"=="good" GOTO CFLAGS
  IF "%pkg%"=="bad" GOTO CFLAGS
)
IF "%mode%"=="libs" (
  IF "%pkg%"=="good" GOTO GOOD_LIBS
  IF "%pkg%"=="bad" GOTO BAD_LIBS
)
EXIT /B 1

:VERSION
ECHO 1.0
ECHO /fake-prefix
EXIT /B 0

:VARIABLE
ECHO /fake-%variable%
EXIT /B 0

:CFLAGS
ECHO -I/fake-include
EXIT /B 0

:GOOD_LIBS
IF "%static%"=="true" (
  ECHO -lgood -ldep
) ELSE (
  ECHO -lgood
)
EXIT /B 0

:BAD_LIBS
IF "%static%"=="true" (
  EXIT /B 1
) ELSE (
  ECHO -lbad
)
EXIT /B 0
