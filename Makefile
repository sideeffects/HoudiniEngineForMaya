# Useful targets:
#     all - build the plugin for the Maya version set by MAYA_VERSION
#     clean - clean up the generated files for the Maya version set by MAYA_VERSION
#     all_maya - build the plugin for all the Maya versions listed in ALL_MAYA_VERSIONS
#     all_maya_clean - clean up the generated files for all the Maya versions
# 
# Useful variables:
#     HFS - directory to the Houdini installation
#     DST_MODULE_DIR - directory for the module description file
#     DST_DIR - directory for the module directories (plug-ins, scripts, etc.)
#     MAKETYPE - setting this variable to "Release" will generate a release build
#     MAYA_VERSION - the Maya version that we're building
#     ALL_MAYA_VERSIONS - a list of all the Maya versions

# detect the OS
UNAME := $(shell uname)
ifneq ($(findstring Linux, $(UNAME)),)
    OS = Linux
else ifneq ($(findstring CYGWIN, $(UNAME)),)
    OS = Cygwin
endif

# output directories
DST_MODULE_DIR = $(HFS)/engine/maya
DST_DIR = $(DST_MODULE_DIR)/maya$(MAYA_VERSION)
DST_PLUG_INS_DIR = $(DST_DIR)/plug-ins
DST_SCRIPTS_DIR = $(DST_DIR)/scripts

LIBNAME = houdiniEngine

ifeq ($(OS), Linux)
    SOSUFFIX = so
else ifeq ($(OS), Cygwin)
    SOSUFFIX = mll
endif
SONAME = $(LIBNAME).$(SOSUFFIX)

# external directories
MAYA_VERSION = 2014
ifeq ($(OS), Linux)
    MAYA_DIR = /usr/autodesk/maya$(MAYA_VERSION)-x64
else ifeq ($(OS), Cygwin)
    # We have to use cygpath to here because Cygwin's make doesn't handle spaces properly
    MAYA_DIR := $(shell cygpath -m -s C:/Program\ Files/Autodesk/Maya$(MAYA_VERSION) 2> /dev/null)

    # each versin of Maya requires a different version of Visual C++
    ifeq ($(MAYA_VERSION), 2014)
	# Visual C++ 2010
	MSVC_SDK := $(shell cygpath -m -s -f "/proc/registry32/HKEY_LOCAL_MACHINE/SOFTWARE/Microsoft/VisualStudio/SxS/VC7/10.0" 2> /dev/null)

	# Windows SDK 7.0: standalone
	# Windows SDK 7.0a: included in Visual Studio 2010
	WIN32_SDK := $(or \
		$(shell cygpath -m -s -f "/proc/registry32/HKEY_LOCAL_MACHINE/SOFTWARE/Microsoft/Microsoft SDKs/Windows/v7.0/InstallationFolder" 2> /dev/null), \
		$(shell cygpath -m -s -f "/proc/registry32/HKEY_LOCAL_MACHINE/SOFTWARE/Microsoft/Microsoft SDKs/Windows/v7.0A/InstallationFolder" 2> /dev/null), \
		)
    else ifeq ($(MAYA_VERSION), 2013.5)
	# Visual C++ 2010
	MSVC_SDK := $(shell cygpath -m -s -f "/proc/registry32/HKEY_LOCAL_MACHINE/SOFTWARE/Microsoft/VisualStudio/SxS/VC7/10.0" 2> /dev/null)

	# Windows SDK 7.0: standalone
	# Windows SDK 7.0a: included in Visual Studio 2010
	WIN32_SDK := $(or \
		$(shell cygpath -m -s -f "/proc/registry32/HKEY_LOCAL_MACHINE/SOFTWARE/Microsoft/Microsoft SDKs/Windows/v7.0/InstallationFolder" 2> /dev/null), \
		$(shell cygpath -m -s -f "/proc/registry32/HKEY_LOCAL_MACHINE/SOFTWARE/Microsoft/Microsoft SDKs/Windows/v7.0A/InstallationFolder" 2> /dev/null), \
		)
    else ifeq ($(MAYA_VERSION), 2013)
	# Visual C++ 2010
	MSVC_SDK := $(shell cygpath -m -s -f "/proc/registry32/HKEY_LOCAL_MACHINE/SOFTWARE/Microsoft/VisualStudio/SxS/VC7/10.0" 2> /dev/null)

	# Windows SDK 7.0: standalone
	# Windows SDK 7.0a: included in Visual Studio 2010
	WIN32_SDK := $(or \
		$(shell cygpath -m -s -f "/proc/registry32/HKEY_LOCAL_MACHINE/SOFTWARE/Microsoft/Microsoft SDKs/Windows/v7.0/InstallationFolder" 2> /dev/null), \
		$(shell cygpath -m -s -f "/proc/registry32/HKEY_LOCAL_MACHINE/SOFTWARE/Microsoft/Microsoft SDKs/Windows/v7.0A/InstallationFolder" 2> /dev/null), \
		)
    else ifeq ($(MAYA_VERSION), 2012)
	# Visual C++ 2008
	MSVC_SDK := $(shell cygpath -m -s -f "/proc/registry32/HKEY_LOCAL_MACHINE/SOFTWARE/Microsoft/VisualStudio/SxS/VC7/9.0" 2> /dev/null)

	# Windows SDK 6.0: standalone
	# Windows SDK 6.0a: included in Visual Studio 2008
	WIN32_SDK := $(or \
		$(shell cygpath -m -s -f "/proc/registry32/HKEY_LOCAL_MACHINE/SOFTWARE/Microsoft/Microsoft SDKs/Windows/v6.0/InstallationFolder" 2> /dev/null), \
		$(shell cygpath -m -s -f "/proc/registry32/HKEY_LOCAL_MACHINE/SOFTWARE/Microsoft/Microsoft SDKs/Windows/v6.0A/InstallationFolder" 2> /dev/null), \
		)
    endif

    # Visual C++
    MSVC_SDK_BIN = $(MSVC_SDK)/bin/amd64
    MSVC_SDK_INCLUDE = $(MSVC_SDK)/include
    MSVC_SDK_LIB = $(MSVC_SDK)/lib/amd64

    # Windows SDK
    WIN32_SDK_INCLUDE = $(WIN32_SDK)/Include
    WIN32_SDK_LIB = $(WIN32_SDK)/Lib/x64
endif

# tools
ifeq ($(OS), Linux)
    CXX ?= g++
    LD = $(CXX)
else ifeq ($(OS), Cygwin)
    CXX = $(MSVC_SDK_BIN)/cl
    LD = $(MSVC_SDK_BIN)/link
endif

# Maya flags
CPPFLAGS += -I$(MAYA_DIR)/include
CPPFLAGS += -D_BOOL -DREQUIRE_IOSTREAM
ifeq ($(OS), Linux)
    LDLIBS += -L$(MAYA_DIR)/lib -lOpenMaya -lOpenMayaFX -lFoundation
else ifeq ($(OS), Cygwin)
    LDLIBS += -LIBPATH:$(MAYA_DIR)/lib OpenMaya.lib OpenMayaFX.lib Foundation.lib
endif

# Houdini flags
CPPFLAGS += -I$(HFS)/custom/houdini/include
CPPFLAGS += -I$(HFS)/toolkit/include
ifeq ($(OS), Linux)
    LDFLAGS += -Wl,-rpath,'$$ORIGIN/../../../../dsolib'
    LDLIBS += -L$(HFS)/dsolib -lHAPI
else ifeq ($(OS), Cygwin)
    LDLIBS += -LIBPATH:$(HFS)/custom/houdini/dsolib libHAPI.a
endif

# general flags
ifeq ($(OS), Linux)
    CXXFLAGS += -Wall

    CXXFLAGS += -m64 -pthread -pipe -fPIC
    LDFLAGS += -Wl,-Bsymbolic
else ifeq ($(OS), Cygwin)
    CXXFLAGS += -nologo
    LDFLAGS += -nologo

    # -- ENABLED WARNINGS --
    # 4101: unreferenced local variable
    # 4996: Deprecation usage
    WARNINGENABLE = -w14101 -w14996
    # -- WARNINGS TREATED AS ERRORS --
    # 4265: 'class' has virtual functions but destructor is not virtual 
    # 4700: uninitialized local variable used
    # 4715: not all control paths return a value
    # 4717: recursive on all control paths, function will cause runtime stack overflow
    # 4263: member func does not override any base class virt member func
    # 4266: no override for virt member func from base 'type'; func is hidden
    # 4390: empty controlled statement found; is this the intent?
    # 4407: cast between different pointer to member representations, compiler may generate incorrect code
    WARNINGERROR = -we4265 -we4700 -we4715 -we4717 -we4263 -we4266 -we4390 \
		   -we4407
    # -- DISABLED WARNINGS --
    # 4100: unreferenced formal parameter
    # 4244: conversion of floating point type possible loss of data
    # 4267: conversion of integer type possible loss of data
    # 4389: signed/unsigned mismatch
    WARNINGDISABLE = -wd4100 -wd4244 -wd4267 -wd4389

    CXXFLAGS += -W4 $(WARNINGENABLE) $(WARNINGERROR) $(WARNINGDISABLE)
    CXXFLAGS += -D_CRT_SECURE_NO_WARNINGS

    CPPFLAGS += -I$(MSVC_SDK_INCLUDE) -I$(WIN32_SDK_INCLUDE)
    CXXFLAGS += -EHsc
    LDLIBS += -LIBPATH:$(MSVC_SDK_LIB) -LIBPATH:$(WIN32_SDK_LIB) -DEFAULTLIB:USER32.lib

    LDFLAGS += -export:initializePlugin -export:uninitializePlugin
endif

# debug/release flags
ifeq ($(OS), Linux)
    ifeq ($(MAKETYPE),)
	CXXFLAGS += -g
    else ifeq ($(MAKETYPE),Release)
	CXXFLAGS += -O3
    endif
else ifeq ($(OS), Cygwin)
    ifeq ($(MAKETYPE),)
	CXXFLAGS += -Od -MDd
    else ifeq ($(MAKETYPE),Release)
	CXXFLAGS += -O2 -MD
    endif
endif

CXXFILES = \
	   AssetNode.C \
	   AssetCommand.C \
	   AssetSubCommand.C \
	   AssetSubCommandLoadAsset.C \
	   AssetSubCommandSync.C \
	   Asset.C \
	   AssetInput.C \
	   AssetInputAsset.C \
	   AssetInputMesh.C \
	   AssetInputCurve.C \
	   AssetInputParticle.C \
	   Object.C \
	   util.C \
	   GeometryObject.C \
	   InstancerObject.C \
	   Geo.C \
	   GeometryPart.C \
	   AssetSyncAttribute.C \
	   AssetSyncOutputObject.C \
	   AssetSyncOutputGeoPart.C \
	   AssetSyncOutputInstance.C \
	   FluidVelocityConvert.C \
	   plugin.C

MELFILES = AEhoudiniAssetTemplate.mel \
	   hapiDebugWindow.mel \
	   houdiniEngineCreateUI.mel \
	   houdiniEngineDeleteUI.mel \
	   houdiniEngineAssetLibraryManager.mel \
	   houdiniEngineUtils.mel \
	   houdiniEngineAssetSync.mel

OBJ_DIR = .obj/maya$(MAYA_VERSION)

OBJFILES = $(patsubst %.C, $(OBJ_DIR)/%.o, $(CXXFILES))

DEPFILES = $(patsubst %.C, $(OBJ_DIR)/%.d, $(CXXFILES))

# This module description file uses relative path to specify the module
# directory. Relative path is only supported by Maya 2013 and newer. The file
# can also be used to set environment variables.
ifneq ($(findstring $(MAYA_VERSION), 2013 2013.5 2014),)
DST_MODULE = $(DST_MODULE_DIR)/houdiniEngine-maya$(MAYA_VERSION)
endif
# This module description file uses absolute path to specify the module
# directory. The installer modifies this path at install time, so that the file
# can be copied to one of Maya's default directory. Depending on the Maya
# version, the file can also be used to set environment variables.
DST_MODULE_ABSOLUTE = $(DST_DIR)/houdiniEngine-maya$(MAYA_VERSION)
DST_PLUG_IN = $(DST_PLUG_INS_DIR)/$(SONAME)
DST_SCRIPTS = $(patsubst %, $(DST_SCRIPTS_DIR)/%, $(MELFILES))

# check build requirement
ifeq ($(OS), Linux)
    CAN_BUILD := $(and \
	    $(realpath $(MAYA_DIR)), \
	    1)
else ifeq ($(OS), Cygwin)
    # On Windows, we don't want to force everyone who has Maya to also build
    # this plugin for now, because building Maya plugin requires a specific
    # Visual C++ version, which may not be installed.
    CAN_BUILD := $(and \
	    $(realpath $(MAYA_DIR)), \
	    $(realpath $(WIN32_SDK)), \
	    $(realpath $(MSVC_SDK)), \
	    1)
endif

.PHONY: all
all:
ifeq ($(CAN_BUILD), 1)
all: $(DST_MODULE) $(DST_MODULE_ABSOLUTE) $(DST_PLUG_IN) $(DST_SCRIPTS)
endif

$(DST_MODULE):
	@mkdir -p $(dir $(@))
	echo "+ MAYAVERSION:$(MAYA_VERSION) houdiniEngine 1.5 maya$(MAYA_VERSION)" > $(@)
ifeq ($(OS), Cygwin)
    ifneq ($(findstring $(MAYA_VERSION), 2013 2013.5 2014),)
        # The module file for Maya 2013 and newer can be used to set environment variables

        # Set the PATH variable for dynamic library.
	echo "PATH +:= ..\\..\\..\\bin" >> $(@)

        ifeq ($(MAYA_VERSION), 2014)
        # Maya 2014 uses python 2.7, and that causes the houdiniEngine plugin
        # to conflict with the Mayatomr (mentalray) plugin. Workaround the
        # problem by forcing Houdini to use python 2.6.
	echo "HOUDINI_PYTHON_VERSION = 2.6" >> $(@)
        endif
    endif
endif

$(DST_MODULE_ABSOLUTE):
	@mkdir -p $(dir $(@))
ifeq ($(OS), Cygwin)
	echo "+ houdiniEngine 1.5 $(shell cygpath -w $(DST_DIR))" > $(@)
    ifneq ($(findstring $(MAYA_VERSION), 2013 2013.5 2014),)
        # The module file for Maya 2013 and newer can be used to set environment variables

        # Set the PATH variable for dynamic library.
	echo "PATH += $(shell cygpath -w $(DST_DIR))\\..\\..\\..\\bin" >> $(@)

        ifeq ($(MAYA_VERSION), 2014)
        # Maya 2014 uses python 2.7, and that causes the houdiniEngine plugin
        # to conflict with the Mayatomr (mentalray) plugin. Workaround the
        # problem by forcing Houdini to use python 2.6.
	echo "HOUDINI_PYTHON_VERSION = 2.6" >> $(@)
        endif
    endif
else
	echo "+ houdiniEngine 1.5 $(DST_DIR)" > $(@)
endif

$(DST_PLUG_IN): $(OBJFILES)
	@mkdir -p $(dir $(@))
ifeq ($(OS), Linux)
	$(LD) -shared $(LDFLAGS) -o $(@) $(OBJFILES) $(LDLIBS)
else ifeq ($(OS), Cygwin)
	$(LD) -DLL $(LDFLAGS) -OUT:$(@) $(OBJFILES) $(LDLIBS)
endif

$(OBJ_DIR)/%.o: %.C
	@mkdir -p $(dir $(@))
ifeq ($(OS), Linux)
	$(CXX) -c -MMD -MP -MT $(@) $(CPPFLAGS) $(CXXFLAGS) -o $(@) $(<)
else ifeq ($(OS), Cygwin)
	$(CXX) -c -showIncludes $(CPPFLAGS) $(CXXFLAGS) -Fo$(@) -Tp$(<) > $(@).log; \
	    compileStatus=$$?; \
	    cat $(@).log | ./clShowIncludesToMake $(@); \
	    rm -f $(@).log; \
	    exit $$compileStatus
endif

$(DST_SCRIPTS_DIR)/%.mel: %.mel
	@mkdir -p $(dir $(@))
	cp $(<) $(@)

-include $(DEPFILES)

# clean
.PHONY: clean
clean:
	rm -f $(DST_MODULE) $(DST_MODULE_ABSOLUTE) $(DST_PLUG_IN) $(DST_SCRIPTS)
ifeq ($(OS), Cygwin)
	rm -f $(DST_PLUG_IN:%.$(SOSUFFIX)=%.lib) $(DST_PLUG_IN:%.$(SOSUFFIX)=%.exp)
endif
	rm -f $(OBJFILES)
	rm -f $(DEPFILES)

# build multiple Maya versions
ALL_MAYA_VERSIONS = 2014 \
		2013.5 \
		2013 \
		2012

.PHONY: all_maya all_maya_clean

define MAYA_VERSION_template
.PHONY: maya$(1) maya$(1)_clean

all_maya: maya$(1)
maya$(1):
	$$(MAKE) MAYA_VERSION=$(1)

all_maya_clean: maya$(1)_clean
maya$(1)_clean:
	$$(MAKE) MAYA_VERSION=$(1) clean

endef

$(foreach maya_version, $(ALL_MAYA_VERSIONS), $(eval $(call MAYA_VERSION_template,$(maya_version))))
