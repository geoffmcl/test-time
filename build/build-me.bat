@setlocal

@set TMPINST=0
@set TMPPRJ=test-time
@set TMPSRC=..
@set TMPBGN=%TIME%
@set TMPINS=..\..\3rdParty
@set TMPLOG=bldlog-1.txt

@set TMPOPTS=
@set TMPOPTS=%TMPOPTS% -DCMAKE_INSTALL_PREFIX=%TMPINS%
@REM set TMPOPTS=%TMPOPTS% -DCMAKE_PREFIX_PATH:PATH=%TMP3RD%
@REM set TMPOPTS=%TMPOPTS% -DBUILD_TEST:BOOL=ON
@REM set TMPOPTS=%TMPOPTS% -DZLIB_NAMES=zlibstatic
@REM set TMPOPTS=%TMPOPTS% -DPNG_NAMES=libpng16_static

:RPT
@if "%~1x" == "x" goto GOTCMD
@set TMPOPTS=%TMPOPTS% %1
@shift
@goto RPT
:GOTCMD

@call chkmsvc %TMPPRJ%
@REM call chkbranch localize

@echo Build %DATE% %TIME% > %TMPLOG%

@if NOT EXIST %TMPSRC%\nul goto NOSRC

@echo Build source %TMPSRC%... all output to build log %TMPLOG%
@echo Build source %TMPSRC%... all output to build log %TMPLOG% >> %TMPLOG%

@if NOT EXIST %TMPSRC%\CMakeLists.txt goto NOCM

cmake %TMPSRC% %TMPOPTS% >> %TMPLOG% 2>&1
@if ERRORLEVEL 1 goto ERR1

cmake --build . --config Debug  >> %TMPLOG% 2>&1
@if ERRORLEVEL 1 goto ERR2

cmake --build . --config Release  >> %TMPLOG% 2>&1
@if ERRORLEVEL 1 goto ERR3

@fa4 "***" %TMPLOG%
@call elapsed %TMPBGN%
@echo Appears a successful build... see %TMPLOG%

@echo.
@if "%TMPINST%x" == "0x" (
@echo No install at this time. Set TMPINST=1
@goto END
)

@echo Continue with Release only install? Only Ctrl+c aborts...
@echo.

@pause

@REM cmake --build . --config Debug  --target INSTALL >> %TMPLOG% 2>&1
@REM if ERRORLEVEL 1 goto ERR4

cmake --build . --config Release  --target INSTALL >> %TMPLOG% 2>&1
@if ERRORLEVEL 1 goto ERR5

@fa4 " -- " %TMPLOG%

@call elapsed %TMPBGN%
@echo All done... see %TMPLOG%

@goto END

:NOXD:
@echo ERROR: Can NOT locate %TMP3RD%! Is x: driver setup? *** FIX ME ***
@goto ISERR

:NOSRC
@echo Can NOT locate source %TMPSRC%! *** FIX ME ***
@echo Can NOT locate source %TMPSRC%! *** FIX ME *** >> %TMPLOG%
@goto ISERR

:NOCM
@echo Can NOT locate %TMPSRC%\CMakeLists.txt!
@echo Can NOT locate %TMPSRC%\CMakeLists.txt! >> %TMPLOG%
@goto ISERR

:ERR1
@echo cmake configuration or generations ERROR
@echo cmake configuration or generations ERROR >> %TMPLOG%
@goto ISERR

:ERR2
@echo ERROR: Cmake build Debug FAILED!
@echo ERROR: Cmake build Debug FAILED! >> %TMPLOG%
@goto ISERR

:ERR3
@echo ERROR: Cmake build Release FAILED!
@echo ERROR: Cmake build Release FAILED! >> %TMPLOG%
@goto ISERR

:ERR4
@echo ERROR: Install Debug FAILED!
@echo ERROR: Install Debug  FAILED! >> %TMPLOG%
@goto ISERR

:ERR5
@echo ERROR: Install Release FAILED!
@echo ERROR: Install Release  FAILED! >> %TMPLOG%
@goto ISERR

:ISERR
@echo See %TMPLOG% for details...
@endlocal
@exit /b 1

:END
@endlocal
@exit /b 0

@REM eof
