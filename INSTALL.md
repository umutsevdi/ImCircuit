# Installation

## With Installer
* Installers are located [here](https://github.com/umutsevdi/logic-circuit-simulator-2/releases/tag/v0.01).
* Download the dedicated installer and run.

## Building From the Source

1. Clone the repository.
```sh
git clone https://github.com/umutsevdi/imcircuit.git
cd  logic-circuit-simulator-2
mkdir build
```

### Windows
Windows build requires CMake,
[Visual Studio 2022](https://visualstudio.microsoft.com/downloads/),
[vcpkg](https://vcpkg.io/en/).

2. Install build dependencies.
```bat
    vckpg.exe install
```
3. Compile project using Visual Studio.
4. Copy `build\package\win32\ImCircuit\` to `%LOCALAPPDATA%\Programs\`.
5. Copy files generated at `build\release` to
`%LOCALAPPDATA%\Programs\ImCircuit\bin\`

> [!NOTE]
> To enforce software rendering install(For Virtual Machines)
> also install [opengl32.dll](https://downloads.fdossena.com/geth.php?r=mesa64-latest)
> to the same directory.

## Linux
2. Install the following dependencies:

|Library|Package Name (Fedora)| Package Name (Debian Based)|
|---------|-------|---|
|curl     |libcurl-devel|libcurl4-openssl-dev|
|glfw     |glfw-devel|libglfw3-dev|
|OpenGL   |mesa-libGL-devel|libgl1-mesa-dev|
|libsecret|libsecret-devel|libsecret-1-dev|
| GTK3| |libgtk-3-dev|

2. CMake Installation
```sh
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make 
cpack
```
3. This will generate APT and RPM builds. Run them with your package manager
to install/uninstall.

> [!NOTE]
> You can also install the executable without a package manager.
> To do that copy files found under `build/package/` folder. This directory will
> mirror the root `/` in UNIX. Copy files in the `/usr/`
> to their dedicated locations.
