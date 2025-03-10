@echo off
set BUILD_DIR=cmake-build-debug

if "%~1"=="" (
    echo Usage: run.bat ^<your-script.sc^>
    exit /b 1
)

set SCRIPT_FILE=%~1

if not exist "%SCRIPT_FILE%" (
    echo Error: The script file "%SCRIPT_FILE%" does not exist.
    exit /b 1
)

if exist output.mi del output.mi
if exist output.txt del output.txt

cmake --build %BUILD_DIR%/

%BUILD_DIR%\scmi_compiler "%SCRIPT_FILE%"

:: Check if output.mi exists, if not exit with an error
if not exist output.mi (
    echo Error: Compilation failed, output.mi was not generated.
    exit /b 1
)

java -jar mi-sim-cli.jar output.mi > output.txt

%BUILD_DIR%\scmi_output output.txt