@echo off
set "BUILD_DIR=cmake-build-debug"

if "%~1"=="" (
    echo Usage: run.bat ^<your-script^.sc^>
    exit /b 1
)

set SCRIPT_FILE=%~1

if not exist "%SCRIPT_FILE%" (
    echo Error: The script file '%SCRIPT_FILE%' does not exist.
    exit /b 1
)

cmake --build "%BUILD_DIR%"

"%BUILD_DIR%\scmi_compiler.exe" "%SCRIPT_FILE%" >nul

java -jar mi-sim-cli.jar output.mi > output.txt

"%BUILD_DIR%\scmi_output.exe" output.txt