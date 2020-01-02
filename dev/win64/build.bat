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
set MINGW_PATH=
set SDL2_PATH=D:\Daniel\Projects\ellie\deps\SDL2-2.0.10
set SDL2_INCLUDE=%SDL2_PATH%\include
set SDL2_LIB=%SDL2_PATH%\lib
set ELLIE_PATH=D:\Daniel\Projects\ellie
set PATH=%LLVM_PATH%;%MINGW_PATH%;%SDL2_PATH%;%PATH%
set BaseFilename=ellie

rem
rem
rem

cd %ELLIE_PATH%\win64

set BuildGameOnly=0
set BuildGameOnlyStr="No"
set _BuildGameOnlyArg=%2
if "%_BuildGameOnlyArg%"=="game" (
    set BuildGameOnly=1
    set BuildGameOnlyStr="Yes"
)

set BuildTypeStr=""
set BuildRelease=0
set BuildDebug=0
set BuildClean=0
set _BuildType=0
set _BuildTypeArg=%1
if "%_BuildTypeArg%"=="release" set _BuildType=1
if "%_BuildTypeArg%"=="debug" set _BuildType=2
if "%_BuildTypeArg%"=="both" set _BuildType=3
if "%_BuildTypeArg%"=="clean" set _BuildType=4
if %_BuildType%==1 (
    set BuildTypeStr="Release"
    set BuildRelease=1
) else (
    if %_BuildType%==2 (
        set BuildTypeStr="Debug"
        set BuildDebug=1
    ) else (
        if %_BuildType%==3 (
            set BuildTypeStr="Both"
            set BuildRelease=1
            set BuildDebug=1
        ) else (
            if %_BuildType%==4 (
                set BuildTypeStr="Clean"
                set BuildClean=1
            ) else (
                echo ERROR: Build Type wasn't supplied or is unknown.
                goto USAGE
            )
        )
    )
)

echo Build Type: %BuildTypeStr%
echo Building Game Only: %BuildGameOnlyStr%
echo Base Filename: %BaseFilename%

rem Use pushd to avoid changing the user's working directory.
if exist ..\bin\win64 (
    pushd ..\bin\win64
) else (
    echo ERROR: "..\bin\win64" directory doesn't exist.
    goto END
)

if not "%EllieShell%"=="1" (
    echo ERROR: Must be run from the Ellie Shell for environment setup.
    goto END
)

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

rem NOTE: -isystem is used to avoid warnings/errors from SDL code.
set SDLCompilerFlags=-isystemW:\dev\SDL2-2.0.9-mingw\x86_64-w64-mingw32\include\SDL2
set SDLLinkerFlags=-LW:\dev\SDL2-2.0.9-mingw\x86_64-w64-mingw32\lib -lSDL2main -lSDL2

rem Weverything can be annoying, but I prefer using it and disabling warnings that I don't care about.
set CompilerWarningFlags=-Werror -Weverything -Wno-c++98-compat -Wno-used-but-marked-unused -Wno-missing-prototypes -Wno-unused-macros -Wno-old-style-cast -Wno-c++98-compat-pedantic -Wno-gnu-zero-variadic-macro-arguments -Wno-string-conversion -Wno-covered-switch-default -Wno-unused-parameter -Wno-exit-time-destructors -Wno-global-constructors -Wno-weak-vtables
rem WARNING: \" is required around -D values to actually make them strings.
set CommonCompilerFlags=-target x86_64-pc-windows-gnu -std=c++17 -mwindows %CompilerWarningFlags% -isystemW:\ellie\deps\include -isystemW:\ellie\deps\src
set CommonLinkerFlags=-lmingw32

set CommonEXEFlags=%CommonCompilerFlags% %SDLCompilerFlags% %CommonLinkerFlags% %SDLLinkerFlags%
set CommonDLLFlags=%CommonEXEFlags% -shared

rem WARNING: \" is required around -D values to actually make them strings.
set ReleaseBuildFlags=-O3 -DBUILD_RELEASE=1
set ReleaseBaseFilename=%BaseFilename%
set ReleaseBuildFlags=!ReleaseBuildFlags! -DGAME_FILENAME=\"!ReleaseBaseFilename!.dll\" -DPLATFORM_LOG_FILENAME=\"!ReleaseBaseFilename!.log\"
if %BuildRelease%==1 (
    if %BuildGameOnly%==0 (
        rem NOTE: Linker flags must come after -o (and maybe source files), otherwise you'll get undefined references.
        echo Building Release EXE
        clang++ -o !ReleaseBaseFilename!.exe %EXESource% %CommonEXEFlags% !ReleaseBuildFlags!
    )
    echo Building Release DLL
    clang++ -o !ReleaseBaseFilename!.dll %DLLSource% %CommonDLLFlags% !ReleaseBuildFlags!
)

rem WARNING: \" is required around -D values to actually make them strings.
set DebugBuildFlags=-g -DBUILD_DEBUG=1
set DebugBaseFilename=%BaseFilename%_debug
set DebugBuildFlags=!DebugBuildFlags! -DGAME_FILENAME=\"!DebugBaseFilename!.dll\" -DPLATFORM_LOG_FILENAME=\"!DebugBaseFilename!.log\"
if %BuildDebug%==1 (
    if %BuildGameOnly%==0 (
        rem NOTE: Linker flags must come after -o (and maybe source files), otherwise you'll get undefined references.
        echo Building Debug EXE
        clang++ -o !DebugBaseFilename!.exe %EXESource% %CommonEXEFlags% !DebugBuildFlags!
    )
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
