# ProcMT 2027

ProcMT stands for **Proc**ess **M**agneto**T**elluric data.

It is your **QC** (quality control) tool for your MT data. <br>
Your professional SW should give better results. If ProcMT gives no results, it is questionable if your data is good enough for professional processing.

I was not able to make a complete port of the "older" ProcMT version.
But most - and all important - parts are ported to Qt 6.X

May 2023: changes for Qt 6.5

August 2023: edi files with ZROT and TROT (required by Geotools®)

December 2023: Mac OS port.

June 2026: split and unsplit. 

**The induction arrows seem not to be good.** <br>


## Prerequisites

* Linux: clang++ / gcc compiler with C++ 20 support 
* Apple clang version 12 or above
* tested with MinGW64 compiler & Ninja make, shipped with Qt 6.5
* CMake 3.24 or above (Version is upgrading fast - Cmake is tied to compilers and their features)
* C++ Boost library installed 1.6 or later 
* the Boost libraries do not necessarily to be compiled
* Qt 6.5 or later be installed (on Mac OS: brew install qt)

## Optional
* on Windows Khronos Vulkan is needed in order to avoid error messages [Vulkan](https://vulkan.lunarg.com/sdk/home)
* C++ package Qt 6.5 or later
* OpenGL and Sql libraries from Qt

The predefined FIR decimation coefficients are read from [filter.db](include/filter.db).
CMake installs it beside the executables, like the other SQL databases. At runtime,
ProcMT looks beside the executable first, then in the configured installation's
`bin` directory. Keep this database available when running from a build directory.
The `mtx16` and `mtx16_2s` filters share the `mtx16` table (313 coefficients).
The `mtx64` and `mtx64_3s` filters share the `mtx64` table (1249 coefficients).
In the filter GUI, `64x` selects normal 64-fold decimation; `64x 3s` adds two
extra seconds after normal FIR delay rounding, analogous to `16x 2s` adding one.
At 1024 Hz and above this gives a three-second total start shift; at lower input
rates the normal FIR delay can make the total longer. Command-line selections
are `-filter 64x` and `-filter "64x 3s"` (or `-filter 64x3s`).
The notch, highpass, lowpass and bandpass filters are still calculated as before.

# Build

## Linux

Install cmake, boost, sqlite, qt, db-browser-for-sqlite, Qt6

## MAC OS

```bash
cd $HOME
xcode-select --install
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"~

(echo; echo 'eval "$(/opt/homebrew/bin/brew shellenv)"') >> /Users/bfr/.zprofile
eval "$(/opt/homebrew/bin/brew shellenv)"

brew update
brew install cmake boost sqlite
brew install qt 
# brew install --cask qt-creator
brew install --cask db-browser-for-sqlite
```

## Windows

Environment variables: <br>

* BOOST_ROOT = C:/devel/boost (I rename boost_1_81_0 to boost) <br>

Path

* Path = C:\Qt\6.11.1\mscv2022_64\bin (or your Qt version) <br>
* Path = C:\devel\VulkanSDK\Bin <br>
* Path = C:\windows\system32\OpenSSH\ - maybe needed

System variables:

* VK_SDK_PATH = C:\devel\VulkanSDK\
* Vulkan_INCLUDE_DIR = C:\devel\VulkanSDK\Include
* Vulkan_SDK = C:\devel\VulkanSDK

as you realize, "chaos".

## START

If you are smart - you simply use my settings and do not need to edit anything. <br>
Especially if you run updates from github, you do not need edit your files again and again.

You **always** need a install directory, where the executable is copied to. <br>
**YOU** must have read/write access to this directory.

```bash
sudo mkdir -p /usr/local/procmt
# Linux:
sudo chown -R $USER:$USER /usr/local/procmt
# Mac OS:
sudo chown -R $USER:staff /usr/local/procmt
```

When later rebuilding the project, you can use the same directory again. <br>
Simply go **INTO** the directory and delete all files and subdirectories. <br>
**DO NOT** delete the directory itself.
For the professionals:

```bash
cd /usr/local/procmt
# check with pwd that you are there!
pwd
# and then delete all files and subdirectories
rm -rf *
```
otherwise use your file manager!

To build the local project:

```zsh
mkdir -p $HOME/build
cd $HOME/devel/github_procmt_2027/procmt_2027
zsh cpp_build_release.zsh
```

On Windows, after installing Qt and MSCV 2022:

You can use my *.bat files, located in windows_bat directory. <br>
You need to edit Path, username, and maybe the Qt version. <br>
Then you can use the *.bat files to build the project. <br>
I open "Windows PowerShell for VS 2022" or "Developer Command Prompt for VS 2022" and run the *.bat files from there. When the Terminal opens, you get a message: <br>
`** Visual Studio 2022 Developer Command Prompt v17.7.4 **` <br>
(or similar, depending on your version) <br>
Then you can run the *.bat files. <br>

### Windows release deployment

The MSVC build enables `PROCMT_WINDOWS_SPLIT_DEPLOYMENT` by default. After
running `windows_bat\zwcl_build_release.bat`, run
`windows_bat\zwcl_deploy_release.bat`. Set `QTDIR` if Qt is not installed at
the path configured in the deployment script. Use a clean install directory
when switching from the old flat deployment, and move or remove the existing
`procmt_zip_dir\procmt` staging directory before deploying again; the script
will not delete it.

The staged `procmt` directory contains:

* `bin`: launchers and SQL databases;
* `libexec`: the actual executables and their Qt configuration;
* `lib`: Qt DLLs, the Visual C++ redistributable installer and `plugins`;
* `include`, `doc` and `share`: installed development files and resources.

Run the launchers in `bin`, not the executables in `libexec`. Windows resolves
ordinary DLL imports before application initialization, so the launchers start
the real programs with the sibling `lib` directory on their inherited `PATH`.
Qt finds its plugins using `libexec\qt.conf`. Keep the directory structure
intact when packaging or moving it. On a machine without the Microsoft Visual
C++ runtime, install `lib\vc_redist.x64.exe` before running the programs.
Each launcher is an additional process and
waits for its application; it forwards the command line and exit status.
Set `PROCMT_WINDOWS_SPLIT_DEPLOYMENT=OFF` at CMake configure time to keep the
old executable installation layout; the split deployment script requires it
to be enabled. Linux and macOS installations are unchanged.

## Rebuild

When forcing a rebuild, you simply delete the build directory and start again.

```bash
cd $HOME/build/procmt_2027
rm -rf *
```

# Version 2025

The filter delay is shifted by one point.

So for a 71 length FIR filter at 512 Hz I shift 477 points <br>
(old version was 476 points) <br>
Simply filter your *source* runs again if you want to be compatible in future.

# License

GPL v3<br>
(this is due to some underlying libraries, not decided by me). <br>
So use it as you like, but do not sell it.
