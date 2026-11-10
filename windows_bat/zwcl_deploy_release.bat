@echo off
setlocal EnableExtensions

set "VSBT=%ProgramFiles(x86)%\Microsoft Visual Studio\2022\BuildTools"
call "%VSBT%\VC\Auxiliary\Build\vcvars64.bat"
if errorlevel 1 exit /b 1

set "base=%HOMEDRIVE%%HOMEPATH%\mtxsw\procmt"
set "installdir=%base%\install\procmt"
set "procmt_zip_root=%base%\procmt_zip_dir"
set "procmt_zip_dir=%procmt_zip_root%\procmt"
if not defined QTDIR set "QTDIR=C:\Qt\6.12.0\msvc2022_64"
set "windeployqt=%QTDIR%\bin\windeployqt.exe"
if not exist "%windeployqt%" set "windeployqt=%QTDIR%\bin\windeployqt6.exe"
if not exist "%windeployqt%" (
    echo windeployqt not found in "%QTDIR%\bin".
    exit /b 1
)
if not exist "%installdir%\libexec\qt.conf" (
    echo Split deployment not found in "%installdir%". Build and install with PROCMT_WINDOWS_SPLIT_DEPLOYMENT=ON first.
    exit /b 1
)
for /R "%installdir%\bin" %%f in (*.dll) do (
    if exist "%%~ff" (
        echo Old DLL found: "%%~ff". Use a clean install directory before deploying.
        exit /b 1
    )
)
if exist "%installdir%\bin\vc_redist.x64.exe" (
    echo Old redistributable found in "%installdir%\bin". Use a clean install directory before deploying.
    exit /b 1
)
if exist "%procmt_zip_dir%" (
    echo Deployment destination already exists: "%procmt_zip_dir%"
    echo Move or remove that destination before deploying again.
    exit /b 1
)

mkdir "%procmt_zip_dir%"
if errorlevel 1 exit /b 1
xcopy /E /I /Y "%installdir%\*" "%procmt_zip_dir%\"
if errorlevel 1 exit /b 1
if not exist "%procmt_zip_dir%\lib" mkdir "%procmt_zip_dir%\lib"
if errorlevel 1 exit /b 1
set "PATH=%QTDIR%\bin;%PATH%"
for %%f in ("%procmt_zip_dir%\libexec\*.exe") do (
    "%windeployqt%" --release --compiler-runtime --no-translations --dir "%procmt_zip_dir%\lib" --libdir "%procmt_zip_dir%\lib" --plugindir "%procmt_zip_dir%\lib\plugins" "%%~ff"
    if errorlevel 1 exit /b 1
)
rem These modules may also be loaded dynamically, so they are not always found by scanning imports.
for %%d in (Qt6OpenGL.dll Qt6OpenGLWidgets.dll Qt6PrintSupport.dll Qt6Xml.dll Qt6SvgWidgets.dll Qt6Svg.dll) do (
    if not exist "%QTDIR%\bin\%%d" (
        echo Required Qt module missing: "%QTDIR%\bin\%%d"
        exit /b 1
    )
    copy /Y "%QTDIR%\bin\%%d" "%procmt_zip_dir%\lib\%%d" >nul
    if errorlevel 1 exit /b 1
)
echo Deployment completed: "%procmt_zip_dir%"
echo The directory can be packaged as-is; bin contains the launchers.
set "zipfile=%base%\procmt.zip"
set "deploydir=%procmt_zip_dir%"
set "zipper=C:\Program Files\7-Zip\7z.exe"
if not exist "%zipper%" (
    echo 7-Zip not found at "%zipper%".
    exit /b 1
)
"%zipper%" a -tzip "%zipfile%" "%deploydir%" -mx=9 -mmt
if errorlevel 1 exit /b 1
echo Deployment completed: "%deploydir%"
echo Archive created: "%zipfile%"
exit /b 0

