:<<BATCH


@echo off
setlocal ENABLEDELAYEDEXPANSION

Taskkill /IM "executable.exe" /F  >nul 2>&1

for %%a in (%*) do set "argv[%%a]=1"

IF "%1"=="" (
    echo build and   run in   debug mode: [36mbuild_and_run.bat filename.cpp [0m
    echo build and   run in release mode: [36mbuild_and_run.bat filename.cpp --release[0m
    echo build and debug in     remedyBG: [36mbuild_and_run.bat filename.cpp --debug[0m
    echo build and   run in    ship mode: [36mbuild_and_run.bat filename.cpp --ship[0m
) ELSE (
    IF EXIST "main.obj"       ( del hw.obj         )
    IF EXIST "vc140.pdb"      ( del vc140.pdb      )
    IF EXIST "executable.pdb" ( del executable.pdb )
    IF EXIST "executable.exe" ( del executable.exe )
    IF EXIST "executable.exp" ( del executable.exp )
    IF EXIST "executable.ilk" ( del executable.ilk )
    IF EXIST "executable.lib" ( del executable.lib )

    cls

    set SHIPDEF=
    IF defined argv[--ship] (
        echo [36m[cow] compiling in ship mode[0m
        set SHIPDEF=/DSHIP
        set OPTARG=2
    ) ELSE IF defined argv[--release] (
        echo [36m[cow] compiling in release mode[0m
        set OPTARG=2
    ) ELSE (
        echo [36m[cow] compiling in debug mode[0m
        set OPTARG=d
    )

    cl -O!OPTARG! ^
    /d2FH4- ^
    -W4 -wd4201 -wd4127 ^
    /nologo -fp:except -Z7 -GR- -EHa- -FC ^
    /I .\opengl ^
    /I .\burkardt ^
    /I .\manifold ^
    /EHsc ^
    /MDd ^
    !SHIPDEF! ^
    %1 ^
    /Feexecutable.exe ^
    /link /NODEFAULTLIB:MSVCRT ^
    /LIBPATH:.\opengl ^
    /LIBPATH:.\manifold ^
    OpenGL32.lib user32.lib gdi32.lib shell32.lib vcruntime.lib ^
    glfw3.lib ^
    Clipper2.lib tbb12_debug.lib tbb12.lib manifold.lib manifoldc.lib

    call :setESC

    IF EXIST "executable.exe" (
        IF defined argv[--debug] (
            echo [36m[cow] debugging in remedyBG[0m
            call _windows_debug_remedybg.bat
        ) ELSE (
            echo [36m[cow] running executable[0m
            @echo on
            start executable.exe
        )
    )
)

@echo off
endlocal

IF EXIST "main.obj"       ( del main.obj       )
IF EXIST "executable.ilk" ( del executable.ilk )

:setESC
for /F %%a in ('"prompt $E$S & echo on & for %%b in (1) do rem"') do set "ESC=%%a"
exit /B







BATCH

if [ "$#" -eq 0  ] || ! [ -f "$1" ]; then
    echo "build and run   in   debug mode: [36m./build_and_run.bat main.cpp [0m"
    echo "build and run   in release mode: [36m./build_and_run.bat main.cpp --release[0m"
    echo "build and debug in      VS Code: [36m./build_and_run.bat main.cpp --debug[0m"
else
    if [ -f "executable" ]; then
        rm executable
    fi

    clear

    OPTARG=0
    if [ "$2" = "--release" ]; then
        echo "[36m[cow] building $1 in release mode[0m"
        OPTARG=3
    else
        echo "[36m[cow] building $1 in debug mode[0m"
    fi

    clang++ \
        $1 \
        -o executable \
        -std=c++11 \
        -fno-strict-aliasing \
        -ferror-limit=256 \
        -O$OPTARG \
        -g \
        -Wall -Wextra \
        -Wshadow \
        -Werror=vla \
        -Wno-deprecated-declarations \
        -Wno-missing-braces \
        -Wno-missing-field-initializers \
        -Wno-char-subscripts \
        -Wno-write-strings \
        -L./manifold -I./manifold \
        -L./opengl   -I./opengl   \
        -L./burkardt -I./burkardt \
        -lglfw3 \
        -framework Cocoa -framework OpenGL -framework IOKit \
        -mmacosx-version-min=11.0 -lsdf -lcollider -lcross_section -lquickhull -lpolygon -lClipper2 -ltbb -lmanifold -lmanifoldc \
        -arch arm64 \
        # -arch x86_64 \
        # -mmacosx-version-min=13.5 \
        # -Wno-c++11-narrowing \
        # -ftime-report \

    if [ -f "executable" ]; then
        if [ "$2" = "--debug" ]; then
            echo "[36m[cow] TODO: debugging $1 in Visual Studio Code[0m"
            source _xplat_debug_vscode.bat
        else
            echo "[36m[cow] running executable[0m"
            ./executable
        fi
    fi
fi

