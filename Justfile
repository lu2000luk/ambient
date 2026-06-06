vcpkg_root := env_var('VCPKG_ROOT')

prepare:
    cmake -B build -S . "-DCMAKE_TOOLCHAIN_FILE={{ vcpkg_root }}/scripts/buildsystems/vcpkg.cmake"

build:
    cmake --build build

run:
    cmake --build build
    ./build/MyApp # Remember to change if you rename your executable

[linux]
clean:
    rm -rf build

[windows]
clean:
    rmdir /s /q build

add *PKGS:
    vcpkg add port {{ PKGS }}
