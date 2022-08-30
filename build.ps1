$path = Split-Path -Parent $MyInvocation.MyCommand.Path
Push-Location $path

cmake -B build

if ($?) {
  cmake --build build
}

if ($?) {
  Set-Location ./build
  ctest
  Pop-Location
}

Pop-Location
