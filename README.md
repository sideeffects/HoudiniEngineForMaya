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
* `MAYA_VERSION` - Specifies the Maya version to compile for. (e.g. `2017`, `2016.5`, `2016`,...).

The CMake install step will create a module directory and file.

For example, on Linux:
```
cd ~/HoudiniEngineForMaya
mkdir build-maya2017
cd build-maya2017
cmake \
          -DHoudiniEngine_ROOT=/opt/hfs16.0.633 \
          -DMaya_ROOT=/usr/autodesk/maya2017 \
          -DCMAKE_INSTALL_PREFIX=../install-maya2017 \
          -DMAYA_VERSION=2017 \
          ..
make && make install
```
