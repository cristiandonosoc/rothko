# Rothko

A simple game engine written from scratch.

The phylosophy of Rothko is "EaaL": Engine as a Library. The whole logic of the
game is your own and Rothko only provides common functionality. The way the
application is started, managed and loops is up to you, Rothko has no opinion.

This differs from engines like Unity in which you have to abide to their workflow,
specially using their own UI. Rothko provides a lot of tooling, but in the end
the UI and logic is up to you.

Is a game engine tooling meant for C++ programmers.

## Cross Platform

Currently Rothko supports Windows, Linux and MacOS. MacOS has the caveat of the
graphics API backend. Rothko itself is agnostic to the graphics API used to render,
by using the concepts of RenderBackends. Current only the OpenGL backend has
been fleshed out. This backend uses modern OpenGL (3.2+). If you cannot get a
modern OpenGL context, then you don't get to use Rothko for now :(

A Vulkan backend is planned, which would enable you to use it on MacOS with
MoltenVK.

## About YCM

I use YCM extensively, but every machine has it's own flags that differ a lot,
specially in cross-platform development. For this, I have modified the
`.ycm_extra_conf.py` YCM file to try to import a python file called
`ycm_extra_conf_local.py'. This file should implement a function called
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

## A bit of history

I've been iterating (albeit very slowly) on these libraries/engine for a while.
Every iteration that's worthwhile considering a change takes on another painter
name. Rothko is currently the 4th iteration, though not all projects started
with the purpose of being a general game engine:

1. Renoir: Simple renderer with basic OpenGL instrospection. Used CMake.
2. Picasso: Mainly worked as a shader editor. Similar to how Unity let you
            change uniforms on a menu.
3. Warhol: Generic game engine that handles basics: Abstracting renderer API,
           basic math, memory management, very crude multithreading.
4. Rothko: Cleanup over Warhol, meant to be used as a engine in a non-trivial
           game project that uses this engine as a git submodule.
