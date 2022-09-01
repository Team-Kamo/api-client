$path = Split-Path -Parent $MyInvocation.MyCommand.Path
Push-Location $path

cmake -B build -DOCTANE_API_CLIENT_ENABLE_TESTING=ON

if ($?) {
  cmake --build build
}

if ($?) {
  Set-Location ./build
  ctest
  Pop-Location
}

Pop-Location
