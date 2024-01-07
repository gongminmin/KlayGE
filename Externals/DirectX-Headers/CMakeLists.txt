UpdateExternalLib("DirectX-Headers" "https://github.com/microsoft/DirectX-Headers.git" "980971e835876dc0cde415e8f9bc646e64667bf7")

set(DXHEADERS_BUILD_TEST OFF CACHE BOOL "" FORCE)
set(DXHEADERS_INSTALL OFF CACHE BOOL "" FORCE)
set(DXHEADERS_BUILD_GOOGLE_TEST OFF CACHE BOOL "" FORCE)

add_subdirectory(DirectX-Headers)
