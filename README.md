# Houdini Engine for Maya
Houdini Engine for Maya is a Maya plug-in that allows deep integration of
Houdini technology into Autodesk Maya through the use of Houdini Engine.

This plug-in brings Houdini's powerful and flexible procedural workflow into
Maya through Houdini Digital Assets. Artists can interactively adjust the
asset's parameters inside Maya, and use Maya geometries as an asset's inputs.
Houdini's procedural engine will then "cook" the asset and the results will be
available right inside Maya.

The plug-in is currently released as a public preview. During the experimental
testing period, the Maya plug-in will be available freely to CG artists.

The easiest way for artists to access the plug-in is to download the latest
production build of Houdini or the [FREE Houdini Apprentice Learning
Edition](http://www.sidefx.com/index.php?option=com_download&task=apprentice&Itemid=208)
and install the Maya plug-in along with the Houdini interactive software.
Houdini Digital Assets created in either Houdini or Apprentice can then be
loaded into Maya through the plug-in. A growing library of Digital Assets for
use in Maya will be available at the [Orbolt Smart 3D Asset
Store](http://www.orbolt.com/maya).

For more information:

* [Houdini Engine for Maya](http://www.sidefx.com/maya)
* [FAQ](http://www.sidefx.com/index.php?option=com_content&task=view&id=2616&Itemid=392)
* [SideFX Labs](http://labs.sidefx.com)

For support and reporting bugs:

* [Houdini Engine for Maya forum](http://www.sidefx.com/index.php?option=com_forum&Itemid=172&page=viewforum&f=53)
* [Bug Submission](http://www.sidefx.com/index.php?option=com_content&task=view&id=768&Itemid=239)

## Supported Maya versions
Currently, the supported Maya versions are:

* 2016
* 2015
* 2014
* 2013.5
* 2013
* 2012

The plug-in is supported on Linux, Windows, and Mac OS X.

## Compiling

To get started:

* Fork this repository to your own Github account using the Fork button at the top.
* Clone the forked repository onto your system.

In order to compile and run the plug-in, both Houdini and Maya have to be
installed. Houdini needs to be installed because the plug-in uses the Houdini
Engine API, which is currently distributed together with Houdini. Since the
Houdini Engine API could change between daily builds, the plug-in also needs to
be compiled with a matching Houdini version.  For now, always download and
install the latest available daily build from www.sidefx.com

### Visual Studio Solution (Windows)
On Windows you can build the plugin by launching the Visual Studio Solution
named houdiniEnginePlugin.sln.  You will need Visual Studio 2010.

* Ensure the platform is set to x64.
* Right click on the houdiniEngine project to bring up its properties
* For each of Debug and Release configurations, update the include path
and linker path (The Additional Include Directories and Additional
Library Directories) to the proper version of Houdini installed on your
system.  There is already a path in the project as a place holder -
only the version number need to be updated.
* Go ahead and build!

Finally edit your Maya.env file (eg. \Documents\maya\2014-x64) to include the
following lines:

PATH = %PATH%;C:\Program Files\Side Effects Software\Houdini xx.y.zzz\bin
MAYA_PLUG_IN_PATH = <You Path>\HoudiniEngineForMaya\x64\<Release or Debug>
MAYA_SCRIPT_PATH = <You Path>\HoudiniEngineForMaya

Run Maya!

### GNU Make (Linux, Cygwin on Windows, and Mac OS X)
The code can be compiled through the Makefile using GNU Make. This can also be
done on Windows through the use of Cygwin.

The Makefile can be configured through several variables that are passed to the
`make` command. The most important variables that need to be set are `HFS` and
`DST_MODULE_DIR`. For example:

    make \
        HFS=/opt/hfs13.0.372 \
        DST_MODULE_DIR=~/HoudiniEngineForMaya/build

* `HFS` specifies the Houdini installation to compile against.
* `DST_MODULE_DIR` specifies where to output various files of the Maya module,
such as the plug-in, MEL scripts, and the module description file.

`make` will then create the Maya module that contains all the files necessary
for loading the plug-in into Maya. When running Maya, the module directory
needs to be included into the MAYA\_MODULE\_PATH environment variable, so that
Maya is able to detect the plug-in. For example:

    export MAYA_MODULE_PATH=~/HoudiniEngineForMaya/build

By default, the Makefile compiles against the latest Maya version. The Maya
version to compile against can be changed by setting the `MAYA_VERSION`
variable. For example:

    MAYA_VERSION=2013.5

If Maya is installed at a non-standard location, the location can be specified
by setting the `MAYA_DIR` variable. For example:

    MAYA_DIR=/path/to/autodesk/maya2013.5-x64
