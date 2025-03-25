$ErrorActionPreference = "Stop"

$name = "vendor"
$source_path = "$PSScriptRoot\..\${name}"
$build_path = "$PSScriptRoot\..\temp\build\${name}"
$install_path = "$PSScriptRoot\..\installed"

cmake -DCMAKE_BUILD_TYPE=Debug `
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
