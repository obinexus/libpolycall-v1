@echo off
REM CBLPolyCall Windows Build System v1.1
REM OBINexus Aegis Engineering - COBOL FFI Bridge with Library Architecture
REM Technical Lead: Nnamdi Michael Okpala
REM Waterfall Phase 2: Local Configuration and Independent Library Generation

setlocal enabledelayedexpansion

REM Build configuration
set CBL_PROJECT_NAME=cblpolycall
set CBL_VERSION=1.0.0
set CBL_SRC_DIR=src
set CBL_BUILD_DIR=build
set CBL_TARGET_DIR=target
set CBL_COPYBOOK_DIR=copybooks
set CBL_LIB_DIR=lib
set CBL_CONFIG_DIR=config

REM Colors for output
set GREEN=[92m
set RED=[91m
set YELLOW=[93m
set BLUE=[94m
set MAGENTA=[95m
set NC=[0m

echo.
echo ==================================================================
echo   CBLPolyCall Windows Build System v%CBL_VERSION%
echo   OBINexus Aegis Engineering - COBOL FFI Bridge
echo   Technical Lead: Nnamdi Michael Okpala
echo   Platform: Windows - Library Architecture Implementation
echo ==================================================================
echo.

REM Detect COBOL compiler
where cobc >nul 2>&1
if %errorlevel% == 0 (
    set COBOL_COMPILER=cobc
    echo %BLUE%[BUILD]%NC% GnuCOBOL detected: cobc
) else (
    where cob >nul 2>&1
    if %errorlevel% == 0 (
        set COBOL_COMPILER=cob
        echo %BLUE%[BUILD]%NC% Micro Focus COBOL detected: cob
    ) else (
        echo %RED%[ERROR]%NC% No COBOL compiler found. Please install GnuCOBOL or Micro Focus COBOL
        exit /b 1
    )
)

REM Parse command line arguments
set ACTION=%1
if "%ACTION%"=="" set ACTION=all

if "%ACTION%"=="help" goto :show_help
if "%ACTION%"=="clean" goto :clean_build
if "%ACTION%"=="debug" goto :debug_info
if "%ACTION%"=="all" goto :build_all
if "%ACTION%"=="config" goto :create_config
if "%ACTION%"=="libraries" goto :build_libraries
if "%ACTION%"=="shared" goto :build_shared
if "%ACTION%"=="static" goto :build_static
if "%ACTION%"=="executable" goto :build_executable
if "%ACTION%"=="standalone" goto :build_standalone
if "%ACTION%"=="test" goto :run_tests
if "%ACTION%"=="init" goto :init_project

echo %RED%[ERROR]%NC% Unknown action: %ACTION%
goto :show_help

:init_project
echo %BLUE%[BUILD]%NC% Initializing project structure with library architecture...
if not exist %CBL_SRC_DIR% mkdir %CBL_SRC_DIR%
if not exist %CBL_BUILD_DIR% mkdir %CBL_BUILD_DIR%
if not exist %CBL_TARGET_DIR% mkdir %CBL_TARGET_DIR%
if not exist %CBL_COPYBOOK_DIR% mkdir %CBL_COPYBOOK_DIR%
if not exist %CBL_LIB_DIR% mkdir %CBL_LIB_DIR%
if not exist %CBL_CONFIG_DIR% mkdir %CBL_CONFIG_DIR%
if not exist tests mkdir tests
echo %GREEN%[SUCCESS]%NC% Project structure with library support initialized
goto :eof

:create_config
echo %BLUE%[CONFIG]%NC% Creating comprehensive local readonly GnuCOBOL configuration...
if not exist %CBL_CONFIG_DIR% mkdir %CBL_CONFIG_DIR%

echo # CBLPolyCall Local GnuCOBOL Configuration v1.0 > %CBL_CONFIG_DIR%\default.conf
echo # OBINexus Aegis Engineering - Complete Runtime Environment >> %CBL_CONFIG_DIR%\default.conf
echo # Technical Lead: Nnamdi Michael Okpala >> %CBL_CONFIG_DIR%\default.conf
echo. >> %CBL_CONFIG_DIR%\default.conf
echo name: "cblpolycall-local" >> %CBL_CONFIG_DIR%\default.conf
echo standard-define: 1 >> %CBL_CONFIG_DIR%\default.conf
echo format: fixed >> %CBL_CONFIG_DIR%\default.conf
echo tab-width: 8 >> %CBL_CONFIG_DIR%\default.conf
echo text-column: 72 >> %CBL_CONFIG_DIR%\default.conf
echo word-length: 61 >> %CBL_CONFIG_DIR%\default.conf
echo literal-length: 8191 >> %CBL_CONFIG_DIR%\default.conf
echo numeric-literal-length: 31 >> %CBL_CONFIG_DIR%\default.conf
echo pic-length: 50 >> %CBL_CONFIG_DIR%\default.conf
echo binary-size: 1-2-4-8 >> %CBL_CONFIG_DIR%\default.conf
echo binary-byteorder: big-endian >> %CBL_CONFIG_DIR%\default.conf
echo binary-truncate: yes >> %CBL_CONFIG_DIR%\default.conf
echo defaultbyte: init >> %CBL_CONFIG_DIR%\default.conf
echo filename-mapping: yes >> %CBL_CONFIG_DIR%\default.conf
echo pretty-display: yes >> %CBL_CONFIG_DIR%\default.conf
echo assign-clause: dynamic >> %CBL_CONFIG_DIR%\default.conf
echo screen-section-rules: std >> %CBL_CONFIG_DIR%\default.conf
echo dpc-in-data: xml >> %CBL_CONFIG_DIR%\default.conf
echo subscript-check: max >> %CBL_CONFIG_DIR%\default.conf
echo complex-odo: no >> %CBL_CONFIG_DIR%\default.conf
echo odoslide: no >> %CBL_CONFIG_DIR%\default.conf
echo init-justify: no >> %CBL_CONFIG_DIR%\default.conf
echo indirect-redefines: no >> %CBL_CONFIG_DIR%\default.conf
echo relax-syntax-checks: no >> %CBL_CONFIG_DIR%\default.conf
echo ref-mod-zero-length: yes >> %CBL_CONFIG_DIR%\default.conf
echo relax-level-hierarchy: no >> %CBL_CONFIG_DIR%\default.conf
echo select-working: no >> %CBL_CONFIG_DIR%\default.conf
echo local-implies-recursive: no >> %CBL_CONFIG_DIR%\default.conf
echo sticky-linkage: no >> %CBL_CONFIG_DIR%\default.conf
echo move-ibm: no >> %CBL_CONFIG_DIR%\default.conf
echo perform-osvs: no >> %CBL_CONFIG_DIR%\default.conf
echo arithmetic-osvs: no >> %CBL_CONFIG_DIR%\default.conf
echo constant-folding: yes >> %CBL_CONFIG_DIR%\default.conf
echo hostsign: no >> %CBL_CONFIG_DIR%\default.conf
echo program-name-redefinition: yes >> %CBL_CONFIG_DIR%\default.conf
echo accept-update: no >> %CBL_CONFIG_DIR%\default.conf
echo accept-auto: no >> %CBL_CONFIG_DIR%\default.conf
echo console-is-crt: no >> %CBL_CONFIG_DIR%\default.conf
echo no-echo-means-secure: no >> %CBL_CONFIG_DIR%\default.conf
echo line-col-zero-default: no >> %CBL_CONFIG_DIR%\default.conf
echo display-special-fig-consts: no >> %CBL_CONFIG_DIR%\default.conf
echo binary-comp-1: no >> %CBL_CONFIG_DIR%\default.conf
echo numeric-pointer: no >> %CBL_CONFIG_DIR%\default.conf
echo move-non-numeric-lit-to-numeric-is-zero: no >> %CBL_CONFIG_DIR%\default.conf
echo implicit-assign-dynamic-var: no >> %CBL_CONFIG_DIR%\default.conf
echo device-mnemonics: no >> %CBL_CONFIG_DIR%\default.conf
echo xml-parse-xmlss: no >> %CBL_CONFIG_DIR%\default.conf
echo areacheck: no >> %CBL_CONFIG_DIR%\default.conf
echo comment-paragraphs: skip >> %CBL_CONFIG_DIR%\default.conf
echo control-division: skip >> %CBL_CONFIG_DIR%\default.conf
echo partial-replace-when-literal-src: skip >> %CBL_CONFIG_DIR%\default.conf
echo memory-size-clause: ignore >> %CBL_CONFIG_DIR%\default.conf
echo multiple-file-tape-clause: ignore >> %CBL_CONFIG_DIR%\default.conf
echo label-records-clause: ignore >> %CBL_CONFIG_DIR%\default.conf
echo value-of-clause: ignore >> %CBL_CONFIG_DIR%\default.conf
echo data-records-clause: ignore >> %CBL_CONFIG_DIR%\default.conf
echo top-level-occurs-clause: ignore >> %CBL_CONFIG_DIR%\default.conf
echo same-as-clause: ignore >> %CBL_CONFIG_DIR%\default.conf
echo type-to-clause: ignore >> %CBL_CONFIG_DIR%\default.conf
echo usage-type: ignore >> %CBL_CONFIG_DIR%\default.conf
echo synchronized-clause: ignore >> %CBL_CONFIG_DIR%\default.conf
echo sync-left-right: ignore >> %CBL_CONFIG_DIR%\default.conf
echo special-names-clause: ignore >> %CBL_CONFIG_DIR%\default.conf
echo goto-statement-without-name: ignore >> %CBL_CONFIG_DIR%\default.conf
echo stop-literal-statement: ignore >> %CBL_CONFIG_DIR%\default.conf
echo stop-identifier-statement: ignore >> %CBL_CONFIG_DIR%\default.conf
echo stop-error-statement: ignore >> %CBL_CONFIG_DIR%\default.conf
echo debugging-mode: ignore >> %CBL_CONFIG_DIR%\default.conf
echo use-for-debugging: ignore >> %CBL_CONFIG_DIR%\default.conf
echo padding-character-clause: ignore >> %CBL_CONFIG_DIR%\default.conf
echo next-sentence-phrase: ignore >> %CBL_CONFIG_DIR%\default.conf
echo listing-statements: ignore >> %CBL_CONFIG_DIR%\default.conf
echo title-statement: ignore >> %CBL_CONFIG_DIR%\default.conf
echo entry-statement: ignore >> %CBL_CONFIG_DIR%\default.conf
echo move-noninteger-to-alphanumeric: ignore >> %CBL_CONFIG_DIR%\default.conf
echo move-figurative-constant-to-numeric: ignore >> %CBL_CONFIG_DIR%\default.conf
echo move-figurative-space-to-numeric: ignore >> %CBL_CONFIG_DIR%\default.conf
echo move-figurative-quote-to-numeric: ignore >> %CBL_CONFIG_DIR%\default.conf
echo odo-without-to: ignore >> %CBL_CONFIG_DIR%\default.conf
echo section-segments: ignore >> %CBL_CONFIG_DIR%\default.conf
echo alter-statement: ignore >> %CBL_CONFIG_DIR%\default.conf
echo call-overflow: ignore >> %CBL_CONFIG_DIR%\default.conf
echo numeric-boolean: ignore >> %CBL_CONFIG_DIR%\default.conf
echo hexadecimal-boolean: ignore >> %CBL_CONFIG_DIR%\default.conf
echo national-literals: ignore >> %CBL_CONFIG_DIR%\default.conf
echo hexadecimal-national-literals: ignore >> %CBL_CONFIG_DIR%\default.conf
echo national-character-literals: ignore >> %CBL_CONFIG_DIR%\default.conf
echo hp-octal-literals: ignore >> %CBL_CONFIG_DIR%\default.conf
echo acu-literals: ignore >> %CBL_CONFIG_DIR%\default.conf
echo ebcdic-symbolic-characters: no >> %CBL_CONFIG_DIR%\default.conf
echo word-continuation: ok >> %CBL_CONFIG_DIR%\default.conf
echo not-exception-before-exception: ok >> %CBL_CONFIG_DIR%\default.conf
echo accept-display-extensions: ok >> %CBL_CONFIG_DIR%\default.conf
echo renames-uncommon-levels: ok >> %CBL_CONFIG_DIR%\default.conf
echo larger-redefines: ok >> %CBL_CONFIG_DIR%\default.conf
echo symbolic-constant: ok >> %CBL_CONFIG_DIR%\default.conf
echo constant-78: ok >> %CBL_CONFIG_DIR%\default.conf
echo constant-01: ok >> %CBL_CONFIG_DIR%\default.conf
echo perform-varying-without-by: ok >> %CBL_CONFIG_DIR%\default.conf
echo reference-out-of-declaratives: ok >> %CBL_CONFIG_DIR%\default.conf
echo program-prototypes: ok >> %CBL_CONFIG_DIR%\default.conf
echo call-convention-mnemonic: ok >> %CBL_CONFIG_DIR%\default.conf
echo call-convention-linkage: ok >> %CBL_CONFIG_DIR%\default.conf
echo using-optional: ok >> %CBL_CONFIG_DIR%\default.conf
echo numeric-value-for-edited-item: ok >> %CBL_CONFIG_DIR%\default.conf
echo incorrect-conf-sec-order: ok >> %CBL_CONFIG_DIR%\default.conf
echo define-constant-directive: ok >> %CBL_CONFIG_DIR%\default.conf
echo free-redefines-position: ok >> %CBL_CONFIG_DIR%\default.conf
echo records-mismatch-record-clause: ok >> %CBL_CONFIG_DIR%\default.conf
echo record-delimiter: ok >> %CBL_CONFIG_DIR%\default.conf
echo sequential-delimiters: ok >> %CBL_CONFIG_DIR%\default.conf
echo record-delim-with-fixed-recs: ok >> %CBL_CONFIG_DIR%\default.conf
echo missing-statement: warning >> %CBL_CONFIG_DIR%\default.conf
echo missing-period: warning >> %CBL_CONFIG_DIR%\default.conf
echo zero-length-literals: ok >> %CBL_CONFIG_DIR%\default.conf
echo xml-generate-extra-phrases: ok >> %CBL_CONFIG_DIR%\default.conf
echo continue-after: ok >> %CBL_CONFIG_DIR%\default.conf
echo goto-entry: warning >> %CBL_CONFIG_DIR%\default.conf
echo assign-variable: ok >> %CBL_CONFIG_DIR%\default.conf
echo assign-using-variable: ok >> %CBL_CONFIG_DIR%\default.conf
echo assign-ext-dyn: ok >> %CBL_CONFIG_DIR%\default.conf
echo assign-disk-from: ok >> %CBL_CONFIG_DIR%\default.conf
echo vsam-status: ignore >> %CBL_CONFIG_DIR%\default.conf
echo self-call-recursive: skip >> %CBL_CONFIG_DIR%\default.conf
echo record-contains-depending-clause: unconformable >> %CBL_CONFIG_DIR%\default.conf
echo picture-l: ok >> %CBL_CONFIG_DIR%\default.conf
echo reserved-words: default >> %CBL_CONFIG_DIR%\default.conf

REM Make configuration readonly
attrib +R %CBL_CONFIG_DIR%\default.conf
echo %GREEN%[SUCCESS]%NC% Comprehensive local readonly configuration created: %CBL_CONFIG_DIR%\default.conf
goto :eof

:build_all
echo %BLUE%[BUILD]%NC% Building all components with library architecture...
call :init_project
call :create_config
call :build_libraries
call :build_executable
echo %GREEN%[SUCCESS]%NC% All components built successfully
goto :eof

:build_libraries
echo %BLUE%[BUILD]%NC% Building independent library components...
call :build_shared
call :build_static
echo %GREEN%[SUCCESS]%NC% Library generation completed
goto :eof

:build_shared
echo %BLUE%[BUILD]%NC% Building shared library (.dll)...
if not exist %CBL_LIB_DIR% mkdir %CBL_LIB_DIR%
if not exist %CBL_CONFIG_DIR%\default.conf call :create_config

REM Check if source files exist
if not exist %CBL_SRC_DIR%\POLYCALL.CBL (
    echo %RED%[ERROR]%NC% Bridge source file not found: %CBL_SRC_DIR%\POLYCALL.CBL
    exit /b 1
)

echo %BLUE%[BUILD]%NC% Compiling shared library with local configuration...
%COBOL_COMPILER% -m --conf=%CBL_CONFIG_DIR%\default.conf -I%CBL_COPYBOOK_DIR% -o %CBL_LIB_DIR%\lib%CBL_PROJECT_NAME%.dll %CBL_SRC_DIR%\POLYCALL.CBL

if %errorlevel% == 0 (
    echo %GREEN%[SUCCESS]%NC% Shared library built: %CBL_LIB_DIR%\lib%CBL_PROJECT_NAME%.dll
) else (
    echo %RED%[ERROR]%NC% Shared library build failed with error code %errorlevel%
    exit /b %errorlevel%
)
goto :eof

:build_static
echo %BLUE%[BUILD]%NC% Building static library (.a)...
if not exist %CBL_BUILD_DIR% mkdir %CBL_BUILD_DIR%
if not exist %CBL_LIB_DIR% mkdir %CBL_LIB_DIR%
if not exist %CBL_CONFIG_DIR%\default.conf call :create_config

echo %BLUE%[BUILD]%NC% Compiling object file for static library...
%COBOL_COMPILER% -c --conf=%CBL_CONFIG_DIR%\default.conf -I%CBL_COPYBOOK_DIR% -o %CBL_BUILD_DIR%\POLYCALL.o %CBL_SRC_DIR%\POLYCALL.CBL

if %errorlevel% == 0 (
    echo %BLUE%[BUILD]%NC% Creating static archive...
    ar rcs %CBL_LIB_DIR%\lib%CBL_PROJECT_NAME%.a %CBL_BUILD_DIR%\POLYCALL.o
    if %errorlevel% == 0 (
        echo %GREEN%[SUCCESS]%NC% Static library built: %CBL_LIB_DIR%\lib%CBL_PROJECT_NAME%.a
    ) else (
        echo %RED%[ERROR]%NC% Static library archiving failed
        exit /b %errorlevel%
    )
) else (
    echo %RED%[ERROR]%NC% Object compilation failed with error code %errorlevel%
    exit /b %errorlevel%
)
goto :eof

:build_executable
echo %BLUE%[BUILD]%NC% Building executable with library linkage...
if not exist %CBL_TARGET_DIR% mkdir %CBL_TARGET_DIR%
if not exist %CBL_CONFIG_DIR%\default.conf call :create_config

REM Ensure shared library exists
if not exist %CBL_LIB_DIR%\lib%CBL_PROJECT_NAME%.dll call :build_shared

REM Check if main source exists
if not exist %CBL_SRC_DIR%\MAIN.CBL (
    echo %RED%[ERROR]%NC% Main source file not found: %CBL_SRC_DIR%\MAIN.CBL
    exit /b 1
)

echo %BLUE%[BUILD]%NC% Compiling executable with local library linkage...
%COBOL_COMPILER% -x --conf=%CBL_CONFIG_DIR%\default.conf -I%CBL_COPYBOOK_DIR% -L%CBL_LIB_DIR% -l%CBL_PROJECT_NAME% -o %CBL_TARGET_DIR%\%CBL_PROJECT_NAME%.exe %CBL_SRC_DIR%\MAIN.CBL

if %errorlevel% == 0 (
    echo %GREEN%[SUCCESS]%NC% Executable built with library linkage: %CBL_TARGET_DIR%\%CBL_PROJECT_NAME%.exe
    echo %BLUE%[INFO]%NC% Library dependency: %CBL_LIB_DIR%\lib%CBL_PROJECT_NAME%.dll
) else (
    echo %YELLOW%[RETRY]%NC% Library linkage failed. Attempting standalone build...
    call :build_standalone
)
goto :eof

:build_standalone
echo %BLUE%[BUILD]%NC% Building standalone executable...
if not exist %CBL_TARGET_DIR% mkdir %CBL_TARGET_DIR%
if not exist %CBL_CONFIG_DIR%\default.conf call :create_config

echo %BLUE%[BUILD]%NC% Compiling standalone executable...
%COBOL_COMPILER% -x --conf=%CBL_CONFIG_DIR%\default.conf -I%CBL_COPYBOOK_DIR% -o %CBL_TARGET_DIR%\%CBL_PROJECT_NAME%-standalone.exe %CBL_SRC_DIR%\MAIN.CBL %CBL_SRC_DIR%\POLYCALL.CBL

if %errorlevel% == 0 (
    echo %GREEN%[SUCCESS]%NC% Standalone executable built: %CBL_TARGET_DIR%\%CBL_PROJECT_NAME%-standalone.exe
) else (
    echo %RED%[ERROR]%NC% Standalone build failed with error code %errorlevel%
    exit /b %errorlevel%
)
goto :eof

:clean_build
echo %BLUE%[BUILD]%NC% Cleaning build artifacts...
if exist %CBL_BUILD_DIR% rmdir /s /q %CBL_BUILD_DIR%
if exist %CBL_TARGET_DIR% rmdir /s /q %CBL_TARGET_DIR%
if exist %CBL_LIB_DIR% rmdir /s /q %CBL_LIB_DIR%
if exist %CBL_CONFIG_DIR%\default.conf (
    attrib -R %CBL_CONFIG_DIR%\default.conf
    del %CBL_CONFIG_DIR%\default.conf
)
del /q *.obj *.lst *.o 2>nul
echo %GREEN%[SUCCESS]%NC% Clean completed
goto :eof

:debug_info
echo CBLPolyCall Build Environment Debug Information
echo ==============================================
echo Platform: Windows
echo COBOL Compiler: %COBOL_COMPILER%
echo Project Name: %CBL_PROJECT_NAME%
echo Version: %CBL_VERSION%
echo Source Directory: %CBL_SRC_DIR%
echo Build Directory: %CBL_BUILD_DIR%
echo Target Directory: %CBL_TARGET_DIR%
echo Library Directory: %CBL_LIB_DIR%
echo Configuration Directory: %CBL_CONFIG_DIR%
echo Copybook Directory: %CBL_COPYBOOK_DIR%
echo.
echo Library Architecture:
echo   Shared Library: %CBL_LIB_DIR%\lib%CBL_PROJECT_NAME%.dll
echo   Static Library: %CBL_LIB_DIR%\lib%CBL_PROJECT_NAME%.a
echo   Executable: %CBL_TARGET_DIR%\%CBL_PROJECT_NAME%.exe
echo   Local Config: %CBL_CONFIG_DIR%\default.conf
goto :eof

:run_tests
echo %BLUE%[BUILD]%NC% Running CBLPolyCall tests with library support...
if not exist %CBL_TARGET_DIR%\%CBL_PROJECT_NAME%.exe (
    if not exist %CBL_TARGET_DIR%\%CBL_PROJECT_NAME%-standalone.exe (
        echo %RED%[ERROR]%NC% No executable found. Run 'build.bat all' first.
        exit /b 1
    ) else (
        set TEST_EXECUTABLE=%CBL_TARGET_DIR%\%CBL_PROJECT_NAME%-standalone.exe
    )
) else (
    set TEST_EXECUTABLE=%CBL_TARGET_DIR%\%CBL_PROJECT_NAME%.exe
)

echo %BLUE%[TEST]%NC% Testing executable: %TEST_EXECUTABLE%
if exist "%TEST_EXECUTABLE%" (
    echo %GREEN%[PASS]%NC% Executable found
) else (
    echo %RED%[FAIL]%NC% Executable not found
    exit /b 1
)

echo %BLUE%[TEST]%NC% Testing executable execution with library environment...
REM Set library path for testing
set PATH=%CBL_LIB_DIR%;%PATH%
echo 5 | "%TEST_EXECUTABLE%" >nul 2>&1
if %errorlevel% == 0 (
    echo %GREEN%[PASS]%NC% Executable runs successfully with library support
) else (
    echo %YELLOW%[WARNING]%NC% Executable test completed with exit code %errorlevel%
)

echo %GREEN%[SUCCESS]%NC% Test suite completed
goto :eof

:show_help
echo CBLPolyCall Windows Build System v%CBL_VERSION%
echo Usage: build.bat [action]
echo.
echo Actions:
echo   all        - Build all components (default)
echo   init       - Initialize project structure
echo   config     - Create local readonly configuration
echo   libraries  - Build both shared and static libraries
echo   shared     - Build shared library (.dll)
echo   static     - Build static library (.a)
echo   executable - Build executable with library linkage
echo   standalone - Build standalone executable
echo   clean      - Clean build artifacts
echo   debug      - Show build environment information
echo   test       - Run test suite
echo   help       - Show this help message
echo.
echo Library Architecture:
echo   - Local configuration: config\default.conf (readonly)
echo   - Shared library: lib\libcblpolycall.dll
echo   - Static library: lib\libcblpolycall.a
echo   - Linked executable: target\cblpolycall.exe
echo   - Standalone executable: target\cblpolycall-standalone.exe
echo.
echo Examples:
echo   build.bat all        # Build complete library architecture
echo   build.bat libraries  # Build only libraries
echo   build.bat executable # Build only executable (requires libraries)
echo   build.bat test       # Run tests with library support
echo.
goto :eof
