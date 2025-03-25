$ErrorActionPreference = "Stop"

$extracted_path = "$PSScriptRoot\..\temp\extracted"
$install_path = "$PSScriptRoot\..\installed"

#################################### SDL ######################################
# todo: configure which SDL subsystems and options we actually need

$name = "SDL-release-3.2.8"
$dependency_path = "$PSScriptRoot\..\dependencies\${name}.zip"
$source_path = "${extracted_path}\${name}"
$build_path = "$PSScriptRoot\..\temp\build\${name}"

if (Test-Path -LiteralPath ${source_path})
{
    Remove-Item -Path ${source_path} -Recurse -Force
}
# todo: check whether zip file has already been extracted and is up to date
Expand-Archive -Path "${dependency_path}" -DestinationPath "${extracted_path}"

cmake -DCMAKE_BUILD_TYPE=Debug `
    -DSDL_SHARED_DEFAULT=OFF `
    -DSDL_STATIC=ON `
    -DSDL_SHARED=OFF `
    -DSDL_TEST_LIBRARY=OFF `
    -DSDL_EXAMPLES=OFF `
    -DSDL_INSTALL=ON `
    -DCMAKE_INSTALL_PREFIX="${install_path}" `
    -S"${source_path}" `
    -B"${build_path}" `
    -DCMAKE_C_COMPILER=cl `
    -GNinja
if ($LastExitCode -ne 0)
{
    exit $LastExitCode
}

cmake --build `
    "${build_path}" `
    --config Debug
if ($LastExitCode -ne 0)
{
    exit $LastExitCode
}

cmake --install `
    "${build_path}" `
    --config Debug `
    --prefix "${install_path}"
if ($LastExitCode -ne 0)
{
    exit $LastExitCode
}
