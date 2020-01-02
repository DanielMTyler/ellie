@echo off
rem
rem ==================================
rem Copyright (C) 2020 Daniel M Tyler.
rem   This file is part of Ellie.
rem ==================================
rem

setlocal EnableDelayedExpansion

rem Use pushd to avoid changing the user's working directory.
if exist ..\..\release (
    pushd ..\..\release
) else (
    echo ERROR: "..\..\release" directory doesn't exist.
    goto END
)

rem Release or Debug
set BuildType=%1
rem if /I doesn't allow for else statements.

if /I %BuildType%=="release" (
    set BuildType=0
    echo Running release build.
    start /WAIT ..\..\bin\win64\ellie.exe
)
if /I %BuildType%=="debug" (
    set BuildType=0
    echo Running debug build.
    start /WAIT ..\..\bin\win64\ellie_debug.exe
)
if %BuildType%==0 (
    echo ERROR: Build Type must be supplied in first parameter and be equal to release or debug.
    goto END
)

:END
endlocal
