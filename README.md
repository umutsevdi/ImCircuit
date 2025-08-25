# Logic Circuit Simulator 2

A free and open-source cross-platform Logic Circuit Simulator rewritten in C++. 

> [!WARNING]  
> Currently in progress. Visit the original Logic Circuit Simulator from
> [here](https://github.com/umutsevdi/Logic-Circuit-Simulator).

https://github.com/user-attachments/assets/a770f99d-ed13-40cf-bca7-54b13cea11b8

## Installation

### With Installer
* Installers are located [here](https://github.com/umutsevdi/logic-circuit-simulator-2/releases/tag/v0.01).
* Download the dedicated installer and run.

### Compiling From Source

#### Windows
1. Install build dependencies.
```bat
    vckpg.exe install
```
2. Compile project using Visual Studio.
3. Download [Inno Setup](https://jrsoftware.org/download.php/is.exe?site=2) tool.
4. Generate the installer using Inno Setup.
5. Run.

> [!NOTE]
> You can also install without the Inno Setup Installer.
> Package artifacts will be built to `build\package\win32` and the executable will be build to
> `build\release` directories. 
> 1. Copy `build\package\win32\Logic Circuit Simulator\` to `C:\Program Files\`.
> 2. Copy the executable and DLL files generated at `build\release` to  `C:\Program Files\Logic Circuit Simulator\bin\`

### Linux
1. Install the following dependencies:

|Library|Package Name (Fedora)| Package Name (Debian Based)|
|---------|-------|---|
|curl     |libcurl-devel|libcurl4-openssl-dev|
|glfw     |glfw-devel|libglfw3-dev|
|OpenGL   |mesa-libGL-devel|libgl1-mesa-dev|
|libsecret|libsecret-devel|libsecret-1-dev|

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
