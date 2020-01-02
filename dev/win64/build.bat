@echo off
rem
rem ==================================
rem Copyright (C) 2020 Daniel M Tyler.
rem   This file is part of Ellie.
rem ==================================
rem

rem setlocal to prevent variables from being saved between calls to this script and causing wacky errors.
rem EnableDelayedExpansion to enable execution time parsing of variables.
setlocal EnableDelayedExpansion

set LLVM_PATH=C:\Program Files\LLVM\bin
set MINGW_PATH=C:\Program Files\mingw-w64\x86_64-8.1.0-posix-seh-rt_v6-rev0\mingw64\bin
set BaseFilename=ellie

rem
rem
rem

set ELLIE_PATH=..\..
rem WARNING: MINGW_PATH is required for Clang to find MingW headers/libs/etc.
set PATH=%LLVM_PATH%;%MINGW_PATH%;%PATH%

set DEPS_INCLUDE_PATH=%ELLIE_PATH%\deps\include
set DEPS_LIB_PATH=%ELLIE_PATH%\deps\lib
set DEPS_SRC_PATH=%ELLIE_PATH%\deps\src

if not exist %ELLIE_PATH%\bin (
    echo ERROR: "%ELLIE_PATH%\bin" directory doesn't exist.
    goto USAGE
)

if not exist %ELLIE_PATH%\bin\win64 (
    echo Making directory "%ELLIE_PATH%\bin\win64".
    mkdir %ELLIE_PATH%\bin\win64
)

rem Use pushd to avoid changing the user's working directory.
if exist %ELLIE_PATH%\bin\win64 (
    pushd %ELLIE_PATH%\bin\win64
) else (
    echo ERROR: "%ELLIE_PATH%\bin\win64" directory doesn't exist.
    goto END
)

set BuildTypeStr=""
set BuildRelease=0
set BuildDebug=0
set BuildClean=0
set _BuildType="%1"
if %_BuildType%=="release" (
    set BuildTypeStr="Release"
    set BuildRelease=1
) else (
    if %_BuildType%=="debug" (
        set BuildTypeStr="Debug"
        set BuildDebug=1
    ) else (
        if %_BuildType%=="clean" (
            set BuildTypeStr="Clean"
            set BuildClean=1
        ) else (
            echo ERROR: Build Type wasn't supplied or is unknown.
            goto USAGE
        )
    )
)
echo Build Type: %BuildTypeStr%
echo Base Filename: %BaseFilename%

rem With our unity build, we only need two source files (EXE+DLL).
set EXESource=..\..\src\platform_win64.cpp
set DLLSource=..\..\src\game.cpp

if not exist "%EXESource%" (
    echo ERROR: "%EXESource%" not found.
    goto END
)

if not exist "%DLLSource%" (
    echo ERROR: "%DLLSource%" not found.
    goto END
)

rem Weverything can be annoying, but I prefer using it and disabling warnings that I don't care about.
set CompilerWarningFlags=-Werror -Weverything -Wno-c++98-compat -Wno-used-but-marked-unused -Wno-missing-prototypes -Wno-unused-macros -Wno-old-style-cast -Wno-c++98-compat-pedantic -Wno-gnu-zero-variadic-macro-arguments -Wno-string-conversion -Wno-covered-switch-default -Wno-unused-parameter -Wno-exit-time-destructors -Wno-global-constructors -Wno-weak-vtables
rem WARNING: \" is required around -D values to actually make them strings.
rem NOTE: -isystem is used for SDL2 to avoid warnings/errors.
rem WARNING: Paths should be enclosed in quotes (") to avoid problems with files/folders with spaces in the name.
set CommonCompilerFlags=-target x86_64-pc-windows-gnu -std=c++17 -mwindows %CompilerWarningFlags% -isystem"%DEPS_INCLUDE_PATH%" -isystem"%DEPS_SRC_PATH%" -isystem"%DEPS_INCLUDE_PATH%\SDL2"
set CommonLinkerFlags=-L"%DEPS_LIB_PATH%" -lmingw32 -lSDL2main -lSDL2

set CommonEXEFlags=%CommonCompilerFlags% %CommonLinkerFlags%
set CommonDLLFlags=%CommonEXEFlags% -shared

rem WARNING: \" is required around -D values to actually make them strings.
set ReleaseBuildFlags=-O3 -DBUILD_RELEASE=1
set ReleaseBaseFilename=%BaseFilename%
set ReleaseBuildFlags=!ReleaseBuildFlags! -DGAME_FILENAME=\"!ReleaseBaseFilename!.dll\" -DPLATFORM_LOG_FILENAME=\"!ReleaseBaseFilename!.log\"
if %BuildRelease%==1 (
    rem NOTE: Linker flags must come after -o (and maybe source files), otherwise you'll get undefined references.
    echo Building Release EXE
    clang++ -o !ReleaseBaseFilename!.exe %EXESource% %CommonEXEFlags% !ReleaseBuildFlags!
    
    echo Building Release DLL
    clang++ -o !ReleaseBaseFilename!.dll %DLLSource% %CommonDLLFlags% !ReleaseBuildFlags!
)

rem WARNING: \" is required around -D values to actually make them strings.
set DebugBuildFlags=-g -DBUILD_DEBUG=1
set DebugBaseFilename=%BaseFilename%_debug
set DebugBuildFlags=!DebugBuildFlags! -DGAME_FILENAME=\"!DebugBaseFilename!.dll\" -DPLATFORM_LOG_FILENAME=\"!DebugBaseFilename!.log\"
if %BuildDebug%==1 (
    rem NOTE: Linker flags must come after -o (and maybe source files), otherwise you'll get undefined references.
    echo Building Debug EXE
    clang++ -o !DebugBaseFilename!.exe %EXESource% %CommonEXEFlags% !DebugBuildFlags!
    
    echo Building Debug DLL
    clang++ -o !DebugBaseFilename!.dll %DLLSource% %CommonDLLFlags% !DebugBuildFlags!
)

if %BuildClean%==1 (
    echo Deleting build files, logs, and live copy of game.
    if exist !ReleaseBaseFilename!.exe del !ReleaseBaseFilename!.exe
    if exist !ReleaseBaseFilename!.dll del !ReleaseBaseFilename!.dll
    if exist !ReleaseBaseFilename!.dll.live del !ReleaseBaseFilename!.dll.live
    if exist !ReleaseBaseFilename!.log del !ReleaseBaseFilename!.log

    if exist !DebugBaseFilename!.exe del !DebugBaseFilename!.exe
    if exist !DebugBaseFilename!.dll del !DebugBaseFilename!.dll
    if exist !DebugBaseFilename!.dll.live del !DebugBaseFilename!.dll.live
    if exist !DebugBaseFilename!.log del !DebugBaseFilename!.log
)

goto END

:USAGE
echo Usage: build release/debug/clean

:END
popd
endlocal
