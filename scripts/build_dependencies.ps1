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

######################## SDL_shadercross dependencies ##########################

$name = "DirectX-Headers-980971e835876dc0cde415e8f9bc646e64667bf7"
$dependency_path = "$PSScriptRoot\..\dependencies\${name}.zip"
$source_path = "${extracted_path}\${name}"

if (Test-Path -LiteralPath ${source_path})
{
    Remove-Item -Path ${source_path} -Recurse -Force
}
# todo: check whether zip file has already been extracted and is up to date
Expand-Archive -Path "${dependency_path}" -DestinationPath "${extracted_path}"

$name = "DirectX-Headers"
Rename-Item -Path ${source_path} -NewName ${name}
$dx_headers_source_path = "${extracted_path}\${name}"

$name = "DirectXShaderCompiler-b4711839eb9a87da7c3436d9b212e0492359fbbd"
$dependency_path = "$PSScriptRoot\..\dependencies\${name}.zip"
$source_path = "${extracted_path}\${name}"

if (Test-Path -LiteralPath ${source_path})
{
    Remove-Item -Path ${source_path} -Recurse -Force
}
# todo: check whether zip file has already been extracted and is up to date
Expand-Archive -Path "${dependency_path}" -DestinationPath "${extracted_path}"

$name = "DirectXShaderCompiler"
Rename-Item -Path ${source_path} -NewName ${name}
$dxc_source_path = "${extracted_path}\${name}"

$name = "SPIRV-Cross-d1d4adbefd411fc4721a2fece15a7f4aaa3dcdfa"
$dependency_path = "$PSScriptRoot\..\dependencies\${name}.zip"
$source_path = "${extracted_path}\${name}"

if (Test-Path -LiteralPath ${source_path})
{
    Remove-Item -Path ${source_path} -Recurse -Force
}
# todo: check whether zip file has already been extracted and is up to date
Expand-Archive -Path "${dependency_path}" -DestinationPath "${extracted_path}"

$name = "SPIRV-Cross"
Rename-Item -Path ${source_path} -NewName ${name}
$spirv_cross_source_path = "${extracted_path}\${name}"

$name = "SPIRV-Headers-3f17b2af6784bfa2c5aa5dbb8e0e74a607dd8b3b"
$dependency_path = "$PSScriptRoot\..\dependencies\${name}.zip"
$source_path = "${extracted_path}\${name}"

if (Test-Path -LiteralPath ${source_path})
{
    Remove-Item -Path ${source_path} -Recurse -Force
}
# todo: check whether zip file has already been extracted and is up to date
Expand-Archive -Path "${dependency_path}" -DestinationPath "${extracted_path}"

$name = "SPIRV-Headers"
Rename-Item -Path ${source_path} -NewName ${name}
$spirv_headers_source_path = "${extracted_path}\${name}"

$name = "SPIRV-Tools-4d2f0b40bfe290dea6c6904dafdf7fd8328ba346"
$dependency_path = "$PSScriptRoot\..\dependencies\${name}.zip"
$source_path = "${extracted_path}\${name}"

if (Test-Path -LiteralPath ${source_path})
{
    Remove-Item -Path ${source_path} -Recurse -Force
}
# todo: check whether zip file has already been extracted and is up to date
Expand-Archive -Path "${dependency_path}" -DestinationPath "${extracted_path}"

$name = "SPIRV-Tools"
Rename-Item -Path ${source_path} -NewName ${name}
$spirv_tools_source_path = "${extracted_path}\${name}"

############################## SDL_shadercross ################################

$name = "SDL_shadercross-main"
$dependency_path = "$PSScriptRoot\..\dependencies\${name}.zip"
$source_path = "${extracted_path}\${name}"
$build_path = "$PSScriptRoot\..\temp\build\${name}"

if (Test-Path -LiteralPath ${source_path})
{
    Remove-Item -Path ${source_path} -Recurse -Force
}
# todo: check whether zip file has already been extracted and is up to date
Expand-Archive -Path "${dependency_path}" -DestinationPath "${extracted_path}"

# Copy/move shadercross dependencies
Remove-Item -Path ${source_path}/external/DirectXShaderCompiler -Recurse -Force
Remove-Item -Path ${source_path}/external/SPIRV-Cross -Recurse -Force
Move-Item -Path ${dxc_source_path} -Destination ${source_path}/external -Force
Move-Item -Path ${spirv_cross_source_path} -Destination ${source_path}/external -Force
Copy-Item -Path ${spirv_headers_source_path} -Destination ${source_path}/external -Recurse -Force
Copy-Item -Path ${spirv_tools_source_path} -Destination ${source_path}/external -Recurse -Force

Remove-Item -Path ${source_path}/external/DirectXShaderCompiler/external/DirectX-Headers -Recurse -Force
Remove-Item -Path ${source_path}/external/DirectXShaderCompiler/external/SPIRV-Headers -Recurse -Force
Remove-Item -Path ${source_path}/external/DirectXShaderCompiler/external/SPIRV-Tools -Recurse -Force
Move-Item -Path ${dx_headers_source_path} -Destination ${source_path}/external/DirectXShaderCompiler/external -Force
Move-Item -Path ${spirv_headers_source_path} -Destination ${source_path}/external/DirectXShaderCompiler/external -Force
Move-Item -Path ${spirv_tools_source_path} -Destination ${source_path}/external/DirectXShaderCompiler/external -Force

# Build and install
cmake -DCMAKE_BUILD_TYPE=Debug `
    -DSDL_SHADERCROSS_DXC=ON `
    -DSDLSHADERCROSS_SHARED=OFF `
    -DSDLSHADERCROSS_STATIC=ON `
    -DSDLSHADERCROSS_SPIRVCROSS_SHARED=ON `
    -DSDLSHADERCROSS_VENDORED=ON `
    -DSDLSHADERCROSS_CLI=ON `
    -DSDLSHADERCROSS_INSTALL=ON `
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

#################################### cglm ######################################

$name = "cglm-0.9.6"
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
    -DCGLM_SHARED=OFF `
    -DCGLM_STATIC=ON `
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
