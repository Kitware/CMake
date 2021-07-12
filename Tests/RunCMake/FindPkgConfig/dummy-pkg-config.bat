@ECHO OFF

rem variables to get around `--static --version` printing the received
rem message and then version
set static=false
set print_errors=false

:LOOP

IF "%1"=="" (
  EXIT /B 255
)

IF "%1"=="--version" (
  ECHO 0.0-cmake-dummy
  EXIT /B 0
)

IF "%1"=="--exists" (
  SHIFT
  ECHO Expected: %*
  ECHO Found:    %PKG_CONFIG_PATH%
  IF NOT "%*"=="%PKG_CONFIG_PATH%" (
    EXIT /B 1
  ) ELSE (
    EXIT /B 0
  )
)
IF "%1"=="--static" (
  set static=true
)
IF "%1"=="--print-errors" (
  set print_errors=true
)
SHIFT
IF NOT "%~1"=="" GOTO LOOP

IF "%static%"=="true" ECHO Received --static
IF "%print_errors%"=="true" ECHO Received --print-errors

IF "%static%"=="true" GOTO :EOF
IF "%print_errors%"=="true" GOTO :EOF

EXIT /B 255
