$ErrorActionPreference = "Stop"

#################################### SDL ######################################
# todo: configure which SDL subsystems and options we actually need

$name = "SDL-release-3.2.8"
$temp_path = "$PSScriptRoot\..\temp\${name}"

if (Test-Path -LiteralPath ${temp_path})
{
    Remove-Item -Path ${temp_path} -Recurse -Force
}
# todo: check whether zip file has already been extracted and is up to date
Expand-Archive -Path "$PSScriptRoot\..\dependencies\${name}.zip" -DestinationPath "$PSScriptRoot\..\temp\extracted"

cmake -DCMAKE_BUILD_TYPE=Debug `
    -DSDL_SHARED_DEFAULT=OFF `
    -DSDL_STATIC=ON `
    -DSDL_SHARED=OFF `
    -DSDL_TEST_LIBRARY=OFF `
    -DSDL_EXAMPLES=OFF `
    -DSDL_INSTALL=ON `
    -DCMAKE_INSTALL_PREFIX="$PSScriptRoot\..\installed" `
    -S"$PSScriptRoot\..\temp\extracted\${name}" `
    -B"$PSScriptRoot\..\temp\build\${name}" `
    -DCMAKE_C_COMPILER=cl `
    -GNinja
if ($LastExitCode -ne 0) {
    exit $LastExitCode
}

cmake --build `
    "$PSScriptRoot\..\temp\build\${name}" `
    --config Debug
if ($LastExitCode -ne 0) {
    exit $LastExitCode
}

cmake --install `
    "$PSScriptRoot\..\temp\build\${name}" `
    --config Debug `
    --prefix "$PSScriptRoot\..\installed"
if ($LastExitCode -ne 0) {
    exit $LastExitCode
}
