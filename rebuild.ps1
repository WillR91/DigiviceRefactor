# PowerShell script to rebuild DigiviceSim project from scratch
Write-Host "Cleaning build directory..."
Remove-Item -Path "build_new" -Recurse -Force -ErrorAction SilentlyContinue
New-Item -Path "build_new" -ItemType Directory -Force | Out-Null

Write-Host "Configuring CMake..."
Set-Location -Path "build_new"
cmake -G "Visual Studio 17 2022" -A x64 ..

Write-Host "Building project..."
cmake --build . --config Debug

Write-Host "Build complete. Check for errors above."
