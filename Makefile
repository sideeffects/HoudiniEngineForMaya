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
else ifneq ($(findstring Darwin, $(UNAME)),)
    OS = Darwin
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
else ifeq ($(OS), Darwin)
    SOSUFFIX = bundle
endif
SONAME = $(LIBNAME).$(SOSUFFIX)

# external directories
MAYA_VERSION = 2014
ifeq ($(OS), Linux)
    ifneq ($(findstring $(MAYA_VERSION), 2012 2013 2013.5 2014 2015),)
	MAYA_DIR = /usr/autodesk/maya$(MAYA_VERSION)-x64
    else ifneq ($(findstring $(MAYA_VERSION), 2016),)
	MAYA_DIR = /usr/autodesk/maya$(MAYA_VERSION)
    endif
else ifeq ($(OS), Cygwin)
    # We have to use cygpath to here because Cygwin's make doesn't handle spaces properly
    MAYA_DIR := $(shell cygpath -m -s C:/Program\ Files/Autodesk/Maya$(MAYA_VERSION) 2> /dev/null)

    # each versin of Maya requires a different version of Visual C++
    ifeq ($(MAYA_VERSION), 2016)
	# Visual C++ 2012
	MSVC_SDK := $(shell cygpath -m -s -f "/proc/registry32/HKEY_LOCAL_MACHINE/SOFTWARE/Microsoft/VisualStudio/SxS/VC7/11.0" 2> /dev/null)

	# Windows SDK 8.0: standalone
	# Windows SDK 8.0a: included in Visual Studio 2012
	WIN32_SDK := $(or \
		$(shell cygpath -m -s -f "/proc/registry32/HKEY_LOCAL_MACHINE/SOFTWARE/Microsoft/Microsoft SDKs/Windows/v8.0/InstallationFolder" 2> /dev/null), \
		$(shell cygpath -m -s -f "/proc/registry32/HKEY_LOCAL_MACHINE/SOFTWARE/Microsoft/Microsoft SDKs/Windows/v8.0A/InstallationFolder" 2> /dev/null), \
		)

	# Windows SDK
	WIN32_SDK_INCLUDE = $(WIN32_SDK)/Include/um \
			    $(WIN32_SDK)/Include/shared
	WIN32_SDK_LIB = $(WIN32_SDK)/Lib/win8/um/x64
    else ifeq ($(MAYA_VERSION), 2015)
	# Visual C++ 2012
	MSVC_SDK := $(shell cygpath -m -s -f "/proc/registry32/HKEY_LOCAL_MACHINE/SOFTWARE/Microsoft/VisualStudio/SxS/VC7/11.0" 2> /dev/null)

	# Windows SDK 8.0: standalone
	# Windows SDK 8.0a: included in Visual Studio 2012
	WIN32_SDK := $(or \
		$(shell cygpath -m -s -f "/proc/registry32/HKEY_LOCAL_MACHINE/SOFTWARE/Microsoft/Microsoft SDKs/Windows/v8.0/InstallationFolder" 2> /dev/null), \
		$(shell cygpath -m -s -f "/proc/registry32/HKEY_LOCAL_MACHINE/SOFTWARE/Microsoft/Microsoft SDKs/Windows/v8.0A/InstallationFolder" 2> /dev/null), \
		)

	# Windows SDK
	WIN32_SDK_INCLUDE = $(WIN32_SDK)/Include/um \
			    $(WIN32_SDK)/Include/shared
	WIN32_SDK_LIB = $(WIN32_SDK)/Lib/win8/um/x64
    else ifeq ($(MAYA_VERSION), 2014)
	# Visual C++ 2010
	MSVC_SDK := $(shell cygpath -m -s -f "/proc/registry32/HKEY_LOCAL_MACHINE/SOFTWARE/Microsoft/VisualStudio/SxS/VC7/10.0" 2> /dev/null)

	# Windows SDK 7.0: standalone
	# Windows SDK 7.0a: included in Visual Studio 2010
	WIN32_SDK := $(or \
		$(shell cygpath -m -s -f "/proc/registry32/HKEY_LOCAL_MACHINE/SOFTWARE/Microsoft/Microsoft SDKs/Windows/v7.0/InstallationFolder" 2> /dev/null), \
		$(shell cygpath -m -s -f "/proc/registry32/HKEY_LOCAL_MACHINE/SOFTWARE/Microsoft/Microsoft SDKs/Windows/v7.0A/InstallationFolder" 2> /dev/null), \
		)

	# Windows SDK
	WIN32_SDK_INCLUDE = $(WIN32_SDK)/Include
	WIN32_SDK_LIB = $(WIN32_SDK)/Lib/x64
    else ifeq ($(MAYA_VERSION), 2013.5)
	# Visual C++ 2010
	MSVC_SDK := $(shell cygpath -m -s -f "/proc/registry32/HKEY_LOCAL_MACHINE/SOFTWARE/Microsoft/VisualStudio/SxS/VC7/10.0" 2> /dev/null)

	# Windows SDK 7.0: standalone
	# Windows SDK 7.0a: included in Visual Studio 2010
	WIN32_SDK := $(or \
		$(shell cygpath -m -s -f "/proc/registry32/HKEY_LOCAL_MACHINE/SOFTWARE/Microsoft/Microsoft SDKs/Windows/v7.0/InstallationFolder" 2> /dev/null), \
		$(shell cygpath -m -s -f "/proc/registry32/HKEY_LOCAL_MACHINE/SOFTWARE/Microsoft/Microsoft SDKs/Windows/v7.0A/InstallationFolder" 2> /dev/null), \
		)

	# Windows SDK
	WIN32_SDK_INCLUDE = $(WIN32_SDK)/Include
	WIN32_SDK_LIB = $(WIN32_SDK)/Lib/x64
    else ifeq ($(MAYA_VERSION), 2013)
	# Visual C++ 2010
	MSVC_SDK := $(shell cygpath -m -s -f "/proc/registry32/HKEY_LOCAL_MACHINE/SOFTWARE/Microsoft/VisualStudio/SxS/VC7/10.0" 2> /dev/null)

	# Windows SDK 7.0: standalone
	# Windows SDK 7.0a: included in Visual Studio 2010
	WIN32_SDK := $(or \
		$(shell cygpath -m -s -f "/proc/registry32/HKEY_LOCAL_MACHINE/SOFTWARE/Microsoft/Microsoft SDKs/Windows/v7.0/InstallationFolder" 2> /dev/null), \
		$(shell cygpath -m -s -f "/proc/registry32/HKEY_LOCAL_MACHINE/SOFTWARE/Microsoft/Microsoft SDKs/Windows/v7.0A/InstallationFolder" 2> /dev/null), \
		)

	# Windows SDK
	WIN32_SDK_INCLUDE = $(WIN32_SDK)/Include
	WIN32_SDK_LIB = $(WIN32_SDK)/Lib/x64
    else ifeq ($(MAYA_VERSION), 2012)
	# Visual C++ 2008
	MSVC_SDK := $(shell cygpath -m -s -f "/proc/registry32/HKEY_LOCAL_MACHINE/SOFTWARE/Microsoft/VisualStudio/SxS/VC7/9.0" 2> /dev/null)

	# Windows SDK 6.0: standalone
	# Windows SDK 6.0a: included in Visual Studio 2008
	WIN32_SDK := $(or \
		$(shell cygpath -m -s -f "/proc/registry32/HKEY_LOCAL_MACHINE/SOFTWARE/Microsoft/Microsoft SDKs/Windows/v6.0/InstallationFolder" 2> /dev/null), \
		$(shell cygpath -m -s -f "/proc/registry32/HKEY_LOCAL_MACHINE/SOFTWARE/Microsoft/Microsoft SDKs/Windows/v6.0A/InstallationFolder" 2> /dev/null), \
		)

	# Windows SDK
	WIN32_SDK_INCLUDE = $(WIN32_SDK)/Include
	WIN32_SDK_LIB = $(WIN32_SDK)/Lib/x64
    endif

    # Visual C++
    MSVC_SDK_BIN = $(MSVC_SDK)/bin/amd64
    MSVC_SDK_INCLUDE = $(MSVC_SDK)/include
    MSVC_SDK_LIB = $(MSVC_SDK)/lib/amd64
else ifeq ($(OS), Darwin)
    MAYA_DIR = /Applications/Autodesk/maya$(MAYA_VERSION)/Maya.app/Contents
endif

# tools
ifeq ($(OS), Linux)
    CXX ?= g++
    LD = $(CXX)
else ifeq ($(OS), Cygwin)
    CXX = $(MSVC_SDK_BIN)/cl
    LD = $(MSVC_SDK_BIN)/link
else ifeq ($(OS), Darwin)
    CXX ?= g++
    LD = $(CXX)
endif

# Maya flags
CPPFLAGS += -D_BOOL -DREQUIRE_IOSTREAM
ifeq ($(OS), Linux)
    CPPFLAGS += -I$(MAYA_DIR)/include
    LDLIBS += -L$(MAYA_DIR)/lib -lOpenMaya -lOpenMayaAnim -lOpenMayaFX -lFoundation
else ifeq ($(OS), Cygwin)
    CPPFLAGS += -I$(MAYA_DIR)/include
    LDLIBS += -LIBPATH:$(MAYA_DIR)/lib OpenMaya.lib OpenMayaAnim.lib OpenMayaFX.lib Foundation.lib
else ifeq ($(OS), Darwin)
    CPPFLAGS += -I$(MAYA_DIR)/../../devkit/include
    LDLIBS += -L$(MAYA_DIR)/MacOS -lOpenMaya -lOpenMayaAnim -lOpenMayaFX -lFoundation
endif

# Houdini flags
CPPFLAGS += -I$(HFS)/custom/houdini/include
CPPFLAGS += -I$(HFS)/toolkit/include
ifeq ($(OS), Linux)
    LDFLAGS += -Wl,-rpath,'$$ORIGIN/../../../../dsolib'
    LDLIBS += -L$(HFS)/dsolib -lHAPIL
else ifeq ($(OS), Cygwin)
    LDLIBS += -LIBPATH:$(HFS)/custom/houdini/dsolib libHAPIL.a
else ifeq ($(OS), Darwin)
    LDFLAGS += -Wl,-rpath,$(HFS)/Frameworks/Houdini.framework/Libraries
    LDLIBS += -L$(HFS)/Frameworks/Houdini.framework/Libraries -lHAPIL    
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

    CPPFLAGS += -I$(MSVC_SDK_INCLUDE) \
		$(foreach include,$(WIN32_SDK_INCLUDE),-I$(include))
    CXXFLAGS += -EHsc
    LDLIBS += -LIBPATH:$(MSVC_SDK_LIB) \
	      $(foreach lib,$(WIN32_SDK_LIB),-LIBPATH:$(lib)) \
	      -DEFAULTLIB:USER32.lib

    LDFLAGS += -export:initializePlugin -export:uninitializePlugin
else ifeq ($(OS), Darwin)
    CPPFLAGS += -DOSMac_ -DOSMac_MachO_
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
	CXXFLAGS += -Od -MDd -Zi
	LDFLAGS += -DEBUG
    else ifeq ($(MAKETYPE),Release)
	CXXFLAGS += -O2 -MD
    endif
else ifeq ($(OS), Darwin)
    ifeq ($(MAKETYPE),)
	CXXFLAGS += -g
    else ifeq ($(MAKETYPE),Release)
	CXXFLAGS += -O3
    endif
endif

CXXFILES = \
	   AssetNode.C \
	   AssetCommand.C \
	   SubCommand.C \
	   AssetSubCommandLoadAsset.C \
	   AssetSubCommandSync.C \
	   EngineCommand.C \
	   Asset.C \
	   Input.C \
	   InputAsset.C \
	   InputMesh.C \
	   InputCurve.C \
	   InputParticle.C \
	   OutputObject.C \
	   util.C \
	   OutputGeometryObject.C \
	   OutputInstancerObject.C \
	   OutputGeometry.C \
	   OutputGeometryPart.C \
	   SyncAttribute.C \
	   SyncOutputObject.C \
	   SyncOutputGeometryPart.C \
	   SyncOutputInstance.C \
	   FluidGridConvert.C \
	   plugin.C \
	   CurveMeshInputNode.C

MELFILES = AEhoudiniAssetTemplate.mel \
	   houdiniEngineCreateUI.mel \
	   houdiniEngineDeleteUI.mel \
	   houdiniEngineAssetLibraryManager.mel \
	   houdiniEnginePreferences.mel \
	   houdiniEngineUtils.mel \
	   houdiniEngineAssetSync.mel

PYFILES = houdini_engine_for_maya/__init__.py \
	  houdini_engine_for_maya/asset_store.py

OBJ_DIR = .obj/maya$(MAYA_VERSION)

OBJFILES = $(patsubst %.C, $(OBJ_DIR)/%.o, $(CXXFILES))

DEPFILES = $(patsubst %.C, $(OBJ_DIR)/%.d, $(CXXFILES))

# This module description file uses relative path to specify the module
# directory. Relative path is only supported by Maya 2013 and newer. The file
# can also be used to set environment variables.
ifneq ($(findstring $(MAYA_VERSION), 2013 2013.5 2014 2015 2016),)
DST_MODULE = $(DST_MODULE_DIR)/houdiniEngine-maya$(MAYA_VERSION)
endif
# This module description file uses absolute path to specify the module
# directory. The installer modifies this path at install time, so that the file
# can be copied to one of Maya's default directory. Depending on the Maya
# version, the file can also be used to set environment variables.
DST_MODULE_ABSOLUTE = $(DST_DIR)/houdiniEngine-maya$(MAYA_VERSION)
DST_PLUG_IN = $(DST_PLUG_INS_DIR)/$(SONAME)
DST_SCRIPTS = $(patsubst %, $(DST_SCRIPTS_DIR)/%, $(MELFILES)) \
	      $(patsubst %, $(DST_SCRIPTS_DIR)/%, $(PYFILES))

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
else ifeq ($(OS), Darwin)
    CAN_BUILD := $(and \
	    $(realpath $(MAYA_DIR)), \
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
ifneq ($(findstring $(MAYA_VERSION), 2013 2013.5 2014 2015 2016),)
    # The module file for Maya 2013 and newer can be used to set environment variables
    ifeq ($(OS), Cygwin)
        # Set the PATH variable for dynamic library.
	echo "PATH +:= ..\\..\\..\\bin" >> $(@)
    endif
	echo "HOUDINI_DSO_EXCLUDE_PATTERN={ROP_OpenGL,COP2_GPULighting,COP2_GPUFog,COP2_GPUEnvironment,COP2_GPUZComposite,COP2_EnableGPU,SHOP_OGL,OBJ_ReLight,VEX_OpRender}*" >> $(@)
endif

$(DST_MODULE_ABSOLUTE):
	@mkdir -p $(dir $(@))
ifeq ($(OS), Cygwin)
	echo "+ houdiniEngine 1.5 $(shell cygpath -w $(DST_DIR))" > $(@)
else
	echo "+ houdiniEngine 1.5 $(DST_DIR)" > $(@)
endif
ifneq ($(findstring $(MAYA_VERSION), 2013 2013.5 2014 2015 2016),)
    # The module file for Maya 2013 and newer can be used to set environment variables
    ifeq ($(OS), Cygwin)
        # Set the PATH variable for dynamic library.
	echo "PATH += $(shell cygpath -w $(DST_DIR))\\..\\..\\..\\bin" >> $(@)
    endif
	echo "HOUDINI_DSO_EXCLUDE_PATTERN={ROP_OpenGL,COP2_GPULighting,COP2_GPUFog,COP2_GPUEnvironment,COP2_GPUZComposite,COP2_EnableGPU,SHOP_OGL,OBJ_ReLight,VEX_OpRender}*" >> $(@)
endif

$(DST_PLUG_IN): $(OBJFILES)
	@mkdir -p $(dir $(@))
ifeq ($(OS), Linux)
	$(LD) -shared $(LDFLAGS) -o $(@) $(OBJFILES) $(LDLIBS)
else ifeq ($(OS), Cygwin)
	$(LD) -DLL $(LDFLAGS) -OUT:$(@) $(OBJFILES) $(LDLIBS)
else ifeq ($(OS), Darwin)
	$(LD) -bundle $(LDFLAGS) -o $(@) $(OBJFILES) $(LDLIBS)
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
else ifeq ($(OS), Darwin)
	$(CXX) -c -MD -MP -MT $(@) $(CPPFLAGS) $(CXXFLAGS) -o $(@) $(<)
endif

$(DST_SCRIPTS_DIR)/%.mel: %.mel
	@mkdir -p $(dir $(@))
	cp $(<) $(@)

$(DST_SCRIPTS_DIR)/%.py: %.py
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
ALL_MAYA_VERSIONS = 2016 \
		2015 \
		2014 \
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
