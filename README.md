# Houdini Engine for Maya
Houdini Engine for Maya is a Maya plug-in that allows deep integration of
Houdini technology into Autodesk Maya through the use of Houdini Engine.

This plug-in brings Houdini's powerful and flexible procedural workflow into
Maya through Houdini Digital Assets. Artists can interactively adjust the
asset's parameters inside Maya, and use Maya geometries as an asset's inputs.
Houdini's procedural engine will then "cook" the asset and the results will be
available right inside Maya.

The easiest way for artists to access the plug-in is to download the latest
production build of Houdini or the [FREE Houdini Apprentice Learning
Edition](https://www.sidefx.com/products/houdini-apprentice/)
and install the Maya plug-in along with the Houdini interactive software.
Houdini Digital Assets created in either Houdini or Apprentice can then be
loaded into Maya through the plug-in. A growing library of Digital Assets for
use in Maya will be available at the [Orbolt Smart 3D Asset
Store](http://www.orbolt.com/maya).

For more information:

* [Houdini Engine for Maya](https://www.sidefx.com/products/houdini-engine/maya-plug-in/)
* [Documentation](http://www.sidefx.com/docs/maya/)
* [FAQ](https://www.sidefx.com/faq/houdini-engine-faq/)

For support and reporting bugs:

* [Houdini Engine for Maya forum](https://www.sidefx.com/forum/46/)
* [Bug Submission](https://www.sidefx.com/bugs/submit/)

## Supported Maya versions
Currently, the supported Maya versions are:

* 2023
* 2022
* 2020
* 2019
* 2018
* 2017
* 2016.5
* 2016
* 2015
* 2014

The plug-in is supported on Linux, Windows, and Mac OS X.

## Compiling with CMake
The Maya plugin uses CMake to compile on all platforms.

To get started:

* Fork this repository to your own Github account using the Fork button at the top.
* Clone the forked repository onto your system.

In order to compile and run the plug-in, both Houdini and Maya have to be
installed. Houdini needs to be installed because the plug-in uses the Houdini
Engine API, which is currently distributed together with Houdini. Since the
Houdini Engine API could change between daily builds, the plug-in also needs to
be compiled with a matching Houdini version.  For now, always download and
install the latest available daily build from www.sidefx.com

The main CMake variables for configuration are:
* `CMAKE_INSTALL_PREFIX` - Specifies the destination directory to create the module directory and files.
* `HoudiniEngine_ROOT` - Specifies the directory of the Houdini installation. (e.g. `$HFS`)
* `Maya_ROOT` - Specifies the directory of the Maya installation. (e.g. `$MAYA_LOCATION`)
* `MAYA_VERSION` - Specifies the Maya version to compile for. (e.g. `2018`, `2017`, `2016.5`, `2016`,...).

The CMake install step will create a module directory and file.

For example, on Linux:
```
cd ~/HoudiniEngineForMaya
mkdir build-maya2018
cd build-maya2018
cmake \
          -DHoudiniEngine_ROOT=/opt/hfs16.0.633 \
          -DMaya_ROOT=/usr/autodesk/maya2018 \
          -DCMAKE_INSTALL_PREFIX=../install-maya2018 \
          -DMAYA_VERSION=2018 \
          ..
make && make install
```

## UI-dependent DSOs

The Maya plug-in disables certain Houdini DSOs by setting the
`HOUDINI_DSO_EXCLUDE_PATTERN` environment variable (see `CMakeLists.txt`).
These Houdini DSOs depend on Houdini's UI libraries, namely:
- `HoudiniUI`
- `HoudiniAPPS1`
- `HoudiniAPPS2`
- `HoudiniAPPS3`

These Houdini UI libraries use Qt, and thus conflict with Maya's Qt libraries.
These Houdini DSOs are excluded to avoid pulling in the Houdini's UI
dependencies.

This same mechanism can be used to exclude your own UI-dependent DSO plugins.

This should only be an issue when using the in-process backend.  If the backend
is loaded with out-of-process backends (named pipe or socket), then it should
be possible to load the UI-dependent DSOs, and load in the UI libraries.

The backend can be set through the Houdini Engine's preferences in Maya. The
following MEL command could also be used to change the preference before
loading the Maya plug-in:
```optionVar -iv "houdiniEngineSessionType" 2 -iv "houdiniEngineSessionPipeCustom" 0;```
