@echo off
SETLOCAL ENABLEDELAYEDEXPANSION

cd ..

REM ---- Arguments ----
SET "REPO_NAME=%~1"
SET "REPO_NAME_SP=%~2"

IF "%REPO_NAME%"=="" (
    ECHO Usage: %0 repo-name "Repo Name"
    EXIT /B 1
)
IF "%REPO_NAME_SP%"=="" (
    ECHO Usage: %0 repo-name "Repo Name"
    EXIT /B 1
)

REM ---- 1. Rename *.pro -> repo-name.pro ----
FOR %%F IN (*.pro) DO (
    IF EXIST "%%F" (
        REN "%%F" "%REPO_NAME%.pro"
        ECHO Renamed %%F to %REPO_NAME%.pro
    )
)

REM ---- 2. Rename *_*.ts -> repo-name_*.ts ----
FOR %%F IN (*_*.ts) DO (
    IF EXIST "%%F" (
        SET "FNAME=%%F"
        FOR /F "tokens=1* delims=_" %%A IN ("!FNAME!") DO (
            SET "SUFFIX=%%B"
            SET "NEW_NAME=!REPO_NAME!_!SUFFIX!"
            REN "%%F" "!NEW_NAME!"
            ECHO Renamed %%F to !NEW_NAME!
        )
    )
)

REM ---- 3. Edit TS file contents ----
FOR %%F IN (!REPO_NAME!_*.ts) DO (
    IF EXIST "%%F" (
        powershell -NoProfile -Command ^
            "$f='%%F';" ^
            "(Get-Content $f) -replace 'language=\".*_en_US\"','language=\"!REPO_NAME!_en_US\"' | Set-Content $f"
        ECHO Updated language in %%F
    )
)

REM ---- 4. Edit only the FIRST PROJECT_NAME in version.pri ----
IF EXIST "version.pri" (
    SET "REPL=PROJECT_NAME            = !REPO_NAME_SP!"
    powershell -NoProfile -Command ^
        "$found=$false; $tmp='version.tmp';" ^
        "Get-Content 'version.pri' | ForEach-Object { if (-not $found -and $_ -match 'PROJECT_NAME\s*=') { $_='!REPL!'; $found=$true }; $_ } | Set-Content $tmp;" ^
        "Move-Item -Force $tmp 'version.pri'"
    ECHO Updated first PROJECT_NAME in version.pri to !REPL!
)

ECHO Done!
PAUSE
