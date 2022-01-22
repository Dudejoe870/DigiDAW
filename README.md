
# DigiDAW

## About

DigiDAW is a Digital Audio Workstation focused on being easy to use, free, and completely open-source.

**[CURRENTLY VERY WORK IN PROGRESS, FEATURES COULD BE COMPLETELY NON-EXISTENT, BUGGY, OR BROKEN. PLEASE BE ADVISED]**

> Note: This software only supports CPUs with AVX support. These are pretty much all CPUs released after 2011.

## Supported Audio Backends

Thanks to the [rtaudio library](https://github.com/thestk/rtaudio), DigiDAW supports a variety of Audio Backends.

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
- [Qt](https://www.qt.io/)
- [rtaudio](https://github.com/thestk/rtaudio) (Included already as a git submodule)

### Linux

- (Audio Backends) Jack/ALSA/PulseAudio

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

### Qt

To install Qt go [here](https://www.qt.io/download-qt-installer).
Go through the installer and make sure to check Qt-6.2.2

After installing, make an environment variable called QTDIR and set it to ``C:\Qt\6.2.2\msvc2019_64``.

Then add to your PATH environment variable ``%QTDIR%/bin`` and ``%QTDIR%/lib``.

### Visual Studio 2022

The next thing you'll want is Visual Studio 2022 (Note: Visual Studio 2019 should also probably work if you'd rather use that, going back further than that however is unsupported), 
to download that click [here](https://visualstudio.microsoft.com/downloads/).

After downloading and installing Visual Studio 2022, open it (making sure that you installed the C++ development component)
you want to click the ``Open Local Folder`` button and navigate to where you cloned the repository.

Once you're here, you should simply be able to click the green arrow at the top of the screen to build and run the application.

## Linux

Linux building hasn't been tested for the current Qt build system. 
It should be possible, but the scripts may need to be altered to work on Linux.

## MacOS (x86 only)

Unfortunately MacOS building hasn't been tested, it should be technically possible,
but the steps to do so won't be discussed here.
