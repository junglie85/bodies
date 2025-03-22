@echo off

setlocal

pushd %~dp0\..
powershell -executionpolicy bypass -file scripts\build_dependencies.ps1

goto :exit

:exit
popd
endlocal

if %errorlevel% NEQ 0 exit /b %errorlevel%
