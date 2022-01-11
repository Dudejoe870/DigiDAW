
# DigiDAW

## About

DigiDAW is a Digital Audio Workstation focused on being easy to use, free, and completely open-source.

**[CURRENTLY VERY WORK IN PROGRESS, FEATURES COULD BE COMPLETELY NON-EXISTENT, BUGGY, OR BROKEN. PLEASE BE ADVISED]**

## Dependencies

### Common

The common dependencies between all platforms are
- [Sciter SDK](https://sciter.com/download/)
- [rtaudio](https://github.com/thestk/rtaudio) (Included already as a git submodule)

### Linux

- GTK3

## Building

> Note: These build instructions are intentionally more detailed and step by step as to enable less experienced people / potentially non-developers 
to try out the application in its early stages, thus if you are more experienced you may have already done some of these steps / already know how to do them.

This section is about building DigiDAW. This is currently the only way of obtaining and using the software as it is very early.

Certain sections of the build instructions can be skipped if you're more experienced / already have a certain component installed.

### Windows

#### Git

To install Git for windows [click here](https://git-scm.com/download/win).
Simply go through the installer step by step, the defaults should be fine.

To clone this repository, choose a new folder where you will store the respository, then right-click, and choose ``Git Bash Here``.
Run the command ```git clone https://github.com/Dudejoe870/DigiDAW.git --recursive ./```
this will clone the git respository into the folder you opened the git bash in.

#### Sciter SDK

After downloading the Sciter SDK and extracting the ZIP somewhere, you must create a new SCITERSDK environment variable and set it to wherever the Sciter SDK is on your drive.

Search ``Environment Variable`` in the start menu, and click on ``Edit the system enviornment variables``.
Click ``Environment Variables``. 
Then under the User variables list, click ``New``.
Make the Variable name ``SCITERSDK``, the Variable value should be the folder path to wherever the Sciter SDK folder is on your drive.

#### Visual Studio 2022

The next thing you'll want is Visual Studio 2022 (Note: Visual Studio 2019 should also probably work if you'd rather use that, going back further than that however is unsupported), 
to download that click [here](https://visualstudio.microsoft.com/downloads/).

After downloading and installing Visual Studio 2022, open it (making sure that you installed the C++ development component)
you want to click the ``Open Local Folder`` button and navigate to where you cloned the repository.

Once you're here, you should simply be able to click the green arrow at the top of the screen to build and run the application.

### Linux

Linux building has been tested on Debian under Windows WSL.

#### Git

If you're on Linux I'll assume you already know how to use a Terminal and navigate things.

Go ahead and clone the repository: ```git clone https://github.com/Dudejoe870/DigiDAW.git --recursive ./```

#### Sciter SDK

After downloading the Sciter SDK and extracting the ZIP somewhere, you must create a new SCITERSDK environment variable and set it to wherever the Sciter SDK is on your drive.

You can opt to put an export command in your bashrc, or simply just export the SCITERSDK enviornment variable before building.

#### GTK3

To install GTK3 on apt based distros, run the command ```sudo apt install libgtk-3-dev```

For pacman based distros, run the command ```sudo pacman -S gtk3```

#### CMake

You'll need CMake to build this application.

For apt based distros it should be ```sudo apt install cmake```.

Or for pacman ```sudo pacman -S cmake```.

#### Other Build Tools

g++ and make are also required, these will normally be pre-installed.

#### Building the Application

To build DigiDAW simply run ```cmake -S ./ -B out/build```
then go inside the out/build folder and run ```make```.

This should build the application. To run it simply go inside the DigiDAW folder inside the build folder and run DigiDAW.

### MacOS (x86 only)

Unfortunately MacOS building hasn't been tested, it should be technically possible,
but the steps to do so won't be discussed here.
