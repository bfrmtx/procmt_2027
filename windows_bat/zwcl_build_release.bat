@echo off
setlocal

call "%~dp0zwcl_config.bat"
if errorlevel 1 exit /b 1

call "%VSBT%\VC\Auxiliary\Build\vcvars64.bat"
if errorlevel 1 exit /b %errorlevel%

if exist "%QTDIR%\bin" set "PATH=%QTDIR%\bin;%PATH%"

if not exist "%PROCMT_BASE%" (
mkdir "%PROCMT_BASE%"
)

set "installdir=%PROCMT_BASE%\install\procmt"

set "srcdir=Z:\github_procmt_2027\procmt_2027"
set "builddir=%PROCMT_BASE%\build\procmt"
if exist "%builddir%\CMakeCache.txt" rmdir /s /q "%builddir%"
cmake -S "%srcdir%" -B "%builddir%" -GNinja -DCMAKE_CXX_COMPILER=cl -DCMAKE_CXX_STANDARD=20 -DCMAKE_INSTALL_PREFIX="%installdir%" -DCMAKE_VERBOSE_MAKEFILE=OFF -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH="%QTDIR%"
if errorlevel 1 exit /b %errorlevel%
cmake --build "%builddir%"
if errorlevel 1 exit /b %errorlevel%
cmake --install "%builddir%"
if errorlevel 1 exit /b %errorlevel%

:deploy_prompt
setlocal EnableDelayedExpansion
set "deploy_answer="
set /p "deploy_answer=Do you want to deploy now [Y/n] "
if not defined deploy_answer (endlocal & goto deploy)
if /i "!deploy_answer!"=="y" (endlocal & goto deploy)
if /i "!deploy_answer!"=="n" (endlocal & exit /b 0)
echo Please enter Y or N.
endlocal
goto deploy_prompt

:deploy
call "%~dp0zwcl_deploy_release.bat"
exit /b %errorlevel%
