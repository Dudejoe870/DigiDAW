
# DigiDAW

## About

DigiDAW is a Digital Audio Workstation focused on being easy to use, free, and completely open-source.

**[CURRENTLY VERY WORK IN PROGRESS, FEATURES COULD BE COMPLETELY NON-EXISTENT, BUGGY, OR BROKEN. PLEASE BE ADVISED]**

>Note: This software compiles with AVX2 support by default. 
If you do not have a CPU from 2013 or later (2015 or later for AMD) then 
you can choose to use only AVX, however if your CPU doesn't support 
even that you can disable SIMD entirely (not recommended) 
using the DIGIDAW_COMPILE_WITH_AVX and DIGIDAW_AVX2 compile options respectively.

## Supported Audio Backends

Thanks to [rtaudio](https://github.com/thestk/rtaudio), DigiDAW supports a variety of Audio Backends.

### Windows

- ASIO
- DirectSound
- WASAPI

#

### Linux

- Jack
- ALSA
- PulseAudio

#

### MacOS

- Core Audio

## Dependencies

### Common

The common dependencies between all platforms are
- OpenGL 3.0
- [GLFW](https://www.glfw.org) (Included already as a git submodule)
- [fmt](https://github.com/fmtlib/fmt) (Included already as a git submodule)
- [ImGUI](https://github.com/ocornut/imgui) (Included already in the source) 
- [ImGUI Knobs](https://github.com/altschuler/imgui-knobs) (Included already in the source)
- [ImGui Addons (gallickgunner)](https://github.com/gallickgunner/ImGui-Addons) (Included already in the source)
- [mIni](https://github.com/pulzed/mINI) (Included already in the source)
- [libsimdpp](https://github.com/p12tic/libsimdpp) (Included already in the source)
- [rtaudio](https://github.com/thestk/rtaudio) (Included already as a git submodule)

### Linux

- [Jack](https://jackaudio.org)/[ALSA](https://www.alsa-project.org/wiki/Main_Page)/[PulseAudio](https://www.freedesktop.org/wiki/Software/PulseAudio/) (At least one of these is required for audio to be functional)

## Building

This section is about building DigiDAW. This is currently the only way of obtaining and using the software as it is very early.

Certain sections of the build instructions can be skipped if you're more experienced / already have a certain component installed.

## Windows

### Git

To install Git for windows [click here](https://git-scm.com/download/win).
Simply go through the installer step by step, the defaults should be fine.

To clone this repository, choose a new folder where you will store the respository, then right-click, and choose ``Git Bash Here``.
Run the command ```git clone https://github.com/Dudejoe870/DigiDAW.git --recursive ./```
this will clone the git respository into the folder you opened the git bash in.

### Visual Studio 2022

The next thing you'll want is Visual Studio 2022 (Note: Visual Studio 2019 should also probably work if you'd rather use that, going back further than that however is unsupported), 
to download that click [here](https://visualstudio.microsoft.com/downloads/).

After downloading and installing Visual Studio 2022, open it (making sure that you installed the C++ development component)
you want to click the ``Open Local Folder`` button and navigate to where you cloned the repository.

Once you're here, you should simply be able to click the green arrow at the top of the screen to build and run the application.

## Linux

Linux building hasn't been tested yet.

## MacOS (x86 only currently)

Unfortunately MacOS building hasn't been tested, it should be technically possible,
but the steps to do so won't be discussed here.
