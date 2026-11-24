@echo off

set "VSBT=%ProgramFiles(x86)%\Microsoft Visual Studio\2022\BuildTools"
if not defined QTDIR set "QTDIR=C:\Qt\6.12.0\msvc2022_64"
if not defined PROCMT_BASE set "PROCMT_BASE=%HOMEDRIVE%%HOMEPATH%\mtxsw\procmt"

exit /b 0
