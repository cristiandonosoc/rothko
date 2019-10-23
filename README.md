# Rothko

A simple game engine written from scratch.
This project is for now a learning experience, though I have used it already for
some minor projects.

The phylosophy of Rothko is "EaaL": Engine as a Library. The whole logic of the
game is your own and Rothko only provides common functionality. The way the
application is started, managed and loops is up to you, Rothko has no opinion.

## Tasks

- Cross-Platform
  - [x] Windows (MSVC)
  - [x] Linux (clang)
  - [x] MacOS (clang, compiled outside XCode)
  - [ ] Android
  - [ ] iPhone
  - [ ] Fuchsia
- Graphics
  - [x] Graphics Agnostic API (Can be implemented by different graphics APIs).
  - [x] OpenGL backend
  - [ ] Vulkan Backend
    - (~20%)I've done some work on this. It works for rendering a static scene but haven't
      done pipeline switch, so it's stuck to a hardcoded shader.
  - [ ] PBR
  - [ ] Shadows
- Window
  - [x] Window Agnositc API
  - [x] SDL integration
- GUI
  - [x] Imgui integration
  - [ ] In game gui
- Assets
  - [ ] GLTF
    - This is ~50% done. Can load models, but there is no material handling yet.
  - [ ] Animations
- Misc
  - [x] C++17
  - [x] Math library
  - [x] Logging (multi-threaded)
  - [x] Scene Graph
  - [x] Debug Widgets (cubes, lines, etc.).
  - [x] Camera support (perspective, rothko, movement, etc.).
  - [ ] Multithreaded Tasks
  - [ ] Memory Management
  - [ ] Containers
- Examples
  - [x] Debug Widgets
  - [x] Scene Graph
  - [x] Simple Lighting
  - [ ] Textured Lighting
  - [ ] Gameboy Emulator
    - On hold because it doesn't really exercide a 3D engine.

## Building

Rothko uses the [GN build system](https://gn.googlesource.com/gn/),
which is used for Chrome and Fuchsia.
Overall the process is pretty simple.

The first time,
```
gn gen out
gn args out
```

In that file, we need to tell GN what dependencies to use for graphics and windows.
```
opengl_enabled = true

// paths are relative to rothko. This is very annoying, yes.
// I have a TODO to make it absolute.
sdl_enabled = true
sdl_include_path = "../../../../../include"
sdl_lib_path = "../../../../../source/SDL2-2.0.10/build/Debug/SDL2d.lib"
```

Finally to compile.
```
ninja -C out
```

On windows, you need to have cl.exe and other tools in the PATH.
The easiest way to do this is to source this script (your actual path depends
on where the Visual Studio installation is):

```
C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars64.bat
```

## Screenshots

A simple lighting experiment
![Simple Lighting](/images/simple_lighting.png)

Some advance on the Gameboy Emulator (on hold right now)
![Emulator](/images/emulator.png)

## About YCM

I use YCM extensively, but every machine has it's own flags that differ a lot,
specially in cross-platform development. For this, I have modified the
`.ycm_extra_conf.py` YCM file to try to import a python file called
`ycm_extra_conf_local.py`. This file should implement a function called
`GetYCMLocalFlags` that should return a python array with the flags YCM style:

```
local_flags = [
  "-I", "/some/path/to/includes",
  "-isystem", "/other/path/to/system/includes",
]

def GetYCMLocalFlags():
  return local_flags
```

This file is not tracked in versioning, so you must provide your own.


