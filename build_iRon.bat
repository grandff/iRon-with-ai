@echo off
REM ==============================================================================
REM Build script for iRon (Windows) with dependency checks
REM ==============================================================================

setlocal EnableDelayedExpansion

REM ---- Helper function to print error and exit ----
:error
echo.
echo [ERROR] %~1
echo.
endlocal
exit /b 1

REM ---- Check for vswhere ----
set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
if not exist "%VSWHERE%" (
    set "VSWHERE=vswhere"
    where vswhere >nul 2>&1 || (
        call :error "vswhere.exe not found. Please install Visual Studio 2017+ or the Build Tools and ensure it is in PATH."
    )
)

echo [INFO] Locating Visual Studio installation...
for /f "usebackq delims=" %%V in (`"%VSWHERE%" -latest -products * -requires Microsoft.Component.MSBuild -find MSBuild\**\Bin\MSBuild.exe`) do set "MSBUILD_PATH=%%V"
if not defined MSBUILD_PATH (
    call :error "Could not locate MSBuild.exe. Ensure Visual Studio with MSBuild component is installed."
)
echo [INFO] Found MSBuild at: %MSBUILD_PATH%

REM ---- Check for required workload (Native Desktop) ----
echo [INFO] Checking for Desktop development with C++ workload...
"%VSWHERE%" -latest -products * -requires Microsoft.VisualStudio.Workload.NativeDesktop >nul 2>&1
if errorlevel 1 (
    call :error "Workload 'Microsoft.VisualStudio.Workload.NativeDesktop' (Desktop development with C++) is not installed. Please modify your Visual Studio installation to include this workload."
)
echo [INFO] Workload check passed.

REM ---- Check for Windows SDK 10.0 ----
echo [INFO] Checking for Windows SDK 10.0...
reg query "HKLM\SOFTWARE\Microsoft\Microsoft SDKs\Windows\v10.0" >nul 2>&1
if errorlevel 1 (
    echo [WARN] Registry check for Windows SDK 10.0 failed. Checking fallback locations...
    set "SDK_INCLUDE_PATH=%ProgramFiles(x86)%\Windows Kits\10\Include\10.0.*.0\um"
    for /d %%I in ("%ProgramFiles(x86)%\Windows Kits\10\Include\*") do (
        if exist "%%I\um" (
            set "SDK_FOUND=1"
        )
    )
    if not defined SDK_FOUND (
        call :error "Windows SDK 10.0 not found. Please install the Windows 10 SDK via Visual Studio Installer."
    )
)
echo [INFO] Windows SDK 10.0 check passed.

REM ---- Optional: Check for iRacing SDK headers (should be in repo) ----
if not exist "irsdk\irsdk_defines.h" (
    call :error "iRacing SDK headers not found in 'irsdk' directory. Ensure you have cloned the repository correctly."
)
echo [INFO] iRacing SDK headers present.

REM ---- Build the project ----
echo [INFO] Starting build...
set "BUILD_CONFIG=Release"
set "BUILD_PLATFORM=x64"

REM Change to script directory
pushd %~dp0

REM Call MSBuild
"%MSBUILD_PATH%" iron.vcxproj /p:Configuration=%BUILD_CONFIG% /p:Platform=%BUILD_PLATFORM% /m
if errorlevel 1 (
    popd
    call :error "MSBuild failed. See output above for details."
)

popd
echo.
echo [SUCCESS] Build completed successfully.
echo [INFO] Output binary: %~dp0Release\iron.exe
echo.
endlocal
exit /b 0