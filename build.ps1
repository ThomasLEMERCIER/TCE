# This script creates a build folder, runs CMake, and changes to the build folder.

# Set your project directory
$PROJECT_DIR = $PSScriptRoot

# Set your build directory
$BUILD_DIR = Join-Path $PROJECT_DIR "build"

# Create the build directory if it doesn't exist
if (-not (Test-Path $BUILD_DIR)) {
    New-Item -Path $BUILD_DIR -ItemType Directory | Out-Null
}

# Change to the build directory
Set-Location -Path $BUILD_DIR

# Run CMake from the build directory
cmake $PROJECT_DIR

# Print a message indicating completion
Write-Host "Build setup complete."
