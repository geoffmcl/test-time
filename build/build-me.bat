@setlocal
@set TMPPRJ=test_time
@set TMPLOG=bldlog-1.txt
@REM 20170722 - Change to msvc140 build
@set VCVERS=14
@set GENERATOR=Visual Studio %VCVERS% Win64
@set SET_BAT=%ProgramFiles(x86)%\Microsoft Visual Studio %VCVERS%.0\VC\vcvarsall.bat
@if NOT EXIST "%SET_BAT%" goto NOBAT
@set TMPROOT=F:\Projects

@set TMPOPTS=
@set TMPOPTS=%TMPOPTS% -G "Visual Studio %VCVERS% Win64"

:RPT
@if "%~1x" == "x" goto GOTCMD
@set TMPOPTS=%TMPOPTS% %1
@shift
@goto RPT
:GOTCMD

@if /I "%PROCESSOR_ARCHITECTURE%" EQU "AMD64" (
@set TMPINST=%TMPROOT%\software.x64
) ELSE (
 @if /I "%PROCESSOR_ARCHITECTURE%" EQU "x86_64" (
@set TMPINST=%TMPROOT%\software.x64
 ) ELSE (
@echo ERROR: Appears 64-bit is NOT available... aborting...
@goto ISERR
 )
)

@call chkmsvc %TMPPRJ%

@echo Build of %TMPPRJ% on %DATE% at %TIME% > %TMPLOG%

@echo Doing: 'call "%SET_BAT%" %PROCESSOR_ARCHITECTURE%'
@echo Doing: 'call "%SET_BAT%" %PROCESSOR_ARCHITECTURE%' >> %TMPLOG%
@call "%SET_BAT%" %PROCESSOR_ARCHITECTURE% >> %TMPLOG% 2>&1
@if ERRORLEVEL 1 goto ERR0

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
