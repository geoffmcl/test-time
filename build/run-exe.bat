@setlocal
@set TMPEXE=Release\test_time.exe
@if EXIST %TMPEXE% goto RUNEXE
@echo Can NOT locate %TMPEXE%! Been built?
@exit /b 1
:RUNEXE
%TMPEXE% %*
