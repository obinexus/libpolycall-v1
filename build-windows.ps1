[CmdletBinding()]
param(
    [ValidateSet("all", "debug", "release", "static", "clean", "rebuild", "help")]
    [string]$Target = "rebuild",

    [string]$Make = "",
    [string]$CC = "gcc",
    [switch]$SkipToolchainCheck
)

$ErrorActionPreference = "Stop"
Set-StrictMode -Version Latest

$ProjectRoot = Split-Path -Parent $MyInvocation.MyCommand.Path

function Resolve-CommandPath {
    param(
        [Parameter(Mandatory = $true)]
        [string[]]$Candidates
    )

    foreach ($Candidate in $Candidates) {
        if ([string]::IsNullOrWhiteSpace($Candidate)) {
            continue
        }

        $Command = Get-Command $Candidate -ErrorAction SilentlyContinue | Select-Object -First 1
        if ($Command) {
            return $Command.Source
        }
    }

    return $null
}

Push-Location $ProjectRoot
try {
    $MakeCandidates = @()
    if ($Make) {
        $MakeCandidates += $Make
    } else {
        $MakeCandidates += "make"
        $MakeCandidates += "mingw32-make"
    }

    $MakePath = Resolve-CommandPath -Candidates $MakeCandidates
    if (-not $MakePath) {
        throw "GNU make was not found. Install MSYS2/MinGW make or pass -Make <path-to-make>."
    }

    $CcPath = Resolve-CommandPath -Candidates @($CC)
    if (-not $CcPath) {
        throw "C compiler '$CC' was not found. Install a MinGW-w64 GCC toolchain or pass -CC <compiler>."
    }

    if (-not $SkipToolchainCheck) {
        $TargetTriple = (& $CcPath -dumpmachine) -join ""
        if ($LASTEXITCODE -ne 0 -or [string]::IsNullOrWhiteSpace($TargetTriple)) {
            throw "Unable to inspect compiler target with '$CC -dumpmachine'."
        }

        if ($TargetTriple -notmatch "mingw|msys|cygwin") {
            throw "Compiler '$CC' targets '$TargetTriple'. Use a MinGW-w64 GCC for the native Windows build."
        }
    }

    $env:OS = "Windows_NT"

    Write-Host "Building libpolycall for Windows with $CC via $MakePath ($Target)"
    & $MakePath $Target "CC=$CC"
    if ($LASTEXITCODE -ne 0) {
        throw "Windows build failed with exit code $LASTEXITCODE."
    }
} finally {
    Pop-Location
}
