@setlocal
@set TMPPRJ=test_time
@set TMPLOG=bldlog-1.txt
@REM 20170722 - Change to msvc140 build
@REM set VCVERS=14
@REM set GENERATOR=Visual Studio %VCVERS% Win64
@REM set SET_BAT=%ProgramFiles(x86)%\Microsoft Visual Studio %VCVERS%.0\VC\vcvarsall.bat
@REM if NOT EXIST "%SET_BAT%" goto NOBAT
@set TMPROOT=D:\UTILS

@set TMPOPTS=
@REM set TMPOPTS=%TMPOPTS% -G "Visual Studio %VCVERS% Win64"

:RPT
@if "%~1x" == "x" goto GOTCMD
@set TMPOPTS=%TMPOPTS% %1
@shift
@goto RPT
:GOTCMD


@call chkmsvc %TMPPRJ%

@echo Build of %TMPPRJ% on %DATE% at %TIME% > %TMPLOG%

@echo Doing: 'cmake .. %TMPOPTS%'
@echo Doing: 'cmake .. %TMPOPTS%' >> %TMPLOG%
@cmake .. %TMPOPTS% >> %TMPLOG% 2>&1
@if ERRORLEVEL 1 goto ERR1

@echo Doing: 'cmake --build . --config Debug'
@echo Doing: 'cmake --build . --config Debug' >> %TMPLOG%
@cmake --build . --config Debug >> %TMPLOG% 2>&1
@if ERRORLEVEL 1 goto ERR2

@echo Doing: 'cmake --build . --config Release'
@echo Doing: 'cmake --build . --config Release' >> %TMPLOG%
@cmake --build . --config Release >> %TMPLOG% 2>&1
@if ERRORLEVEL 1 goto ERR3
@echo.
@echo Appears successful build... see %TMPLOG%
@echo.
@echo No install at this time...
@echo.
@goto END

:ERR0
@echo MSVC setup error
@goto ISERR

:ERR1
@echo ERROR: CMake configure or generation...
@goto ISERR

:ERR2
@echo ERROR: CMake build Debug..
@goto ISERR

:ERR3
@echo ERROR: CMake build Release...
@goto ISERR

:NOBAT
@echo Can NOT locate MSVC setup batch "%SET_BAT%"! *** FIX ME ***
@goto ISERR

:ISERR
@echo See %TMPLOG%
@endlocal
@exit /b 1

:END
@endlocal
@exit /b 0

@REM eof
