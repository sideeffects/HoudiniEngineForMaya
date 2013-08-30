# detect the OS
UNAME := $(shell uname)
ifneq ($(findstring Linux, $(UNAME)),)
    OS = Linux
else ifneq ($(findstring CYGWIN, $(UNAME)),)
    OS = Cygwin
endif

# directory for the module description file
DST_MODULE_DIR = $(HFS)/custom/houdiniEngine/maya
# directory for the module directories (plug-ins, scripts, etc.)
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
	MSVC_SDK := $(shell cygpath -m -s -f "/proc/registry/HKEY_LOCAL_MACHINE/SOFTWARE/Microsoft/VisualStudio/SxS/VC7/10.0" 2> /dev/null)

	# Windows SDK 7.0: standalone
	# Windows SDK 7.0a: included in Visual Studio 2010
	WIN32_SDK := $(or \
		$(shell cygpath -m -s -f "/proc/registry/HKEY_LOCAL_MACHINE/SOFTWARE/Microsoft/Microsoft SDKs/Windows/v7.0/InstallationFolder" 2> /dev/null), \
		$(shell cygpath -m -s -f "/proc/registry/HKEY_LOCAL_MACHINE/SOFTWARE/Microsoft/Microsoft SDKs/Windows/v7.0A/InstallationFolder" 2> /dev/null), \
		)
    else ifeq ($(MAYA_VERSION), 2013)
	# Visual C++ 2010
	MSVC_SDK := $(shell cygpath -m -s -f "/proc/registry/HKEY_LOCAL_MACHINE/SOFTWARE/Microsoft/VisualStudio/SxS/VC7/10.0" 2> /dev/null)

	# Windows SDK 7.0: standalone
	# Windows SDK 7.0a: included in Visual Studio 2010
	WIN32_SDK := $(or \
		$(shell cygpath -m -s -f "/proc/registry/HKEY_LOCAL_MACHINE/SOFTWARE/Microsoft/Microsoft SDKs/Windows/v7.0/InstallationFolder" 2> /dev/null), \
		$(shell cygpath -m -s -f "/proc/registry/HKEY_LOCAL_MACHINE/SOFTWARE/Microsoft/Microsoft SDKs/Windows/v7.0A/InstallationFolder" 2> /dev/null), \
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
    CXX = g++
    LD = g++
else ifeq ($(OS), Cygwin)
    CXX = $(MSVC_SDK_BIN)/cl
    LD = $(MSVC_SDK_BIN)/link
endif

# Maya flags
CPPFLAGS += -I$(MAYA_DIR)/include
CPPFLAGS += -D_BOOL -DREQUIRE_IOSTREAM
ifeq ($(OS), Linux)
    LDLIBS += -L$(MAYA_DIR)/lib -lOpenMaya -lFoundation
else ifeq ($(OS), Cygwin)
    LDLIBS += -LIBPATH:$(MAYA_DIR)/lib OpenMaya.lib Foundation.lib
endif

# Houdini flags
CPPFLAGS += -I$(HFS)/custom/houdini/include
#CPPFLAGS += -I$(HFS)/toolkit/include
ifeq ($(OS), Linux)
    LDFLAGS += -Wl,-rpath,$(HFS)/dsolib
    LDLIBS += -L$(HFS)/dsolib -lHAPI
else ifeq ($(OS), Cygwin)
    LDLIBS += -LIBPATH:$(HFS)/custom/houdini/dsolib libHAPI.a
endif

# general flags
ifeq ($(OS), Linux)
    CXXFLAGS += -m64 -pthread -pipe -fPIC
    LDFLAGS += -Wl,-Bsymbolic
else ifeq ($(OS), Cygwin)
    CXXFLAGS += -nologo
    LDFLAGS += -nologo

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
	   AssetSubCommandLoadOTL.C \
	   AssetSubCommandSync.C \
	   Asset.C \
	   AssetInput.C \
	   AssetInputAsset.C \
	   AssetInputMesh.C \
	   AssetInputCurve.C \
	   Object.C \
	   util.C \
	   GeometryObject.C \
	   InstancerObject.C \
	   GeometryPart.C \
	   AssetSync.C \
	   AssetSyncOutputGeoPart.C \
	   AssetSyncOutputInstance.C \
	   plugin.C

MELFILES = AEhoudiniAssetTemplate.mel \
	   hapiDebugWindow.mel \
	   houdiniEngineCreateUI.mel \
	   houdiniEngineDeleteUI.mel

OBJ_DIR = .obj

OBJFILES = $(patsubst %.C, $(OBJ_DIR)/%.o, $(CXXFILES))

DEPFILES = $(patsubst %.C, $(OBJ_DIR)/%.d, $(CXXFILES))

DST_MODULE = $(DST_MODULE_DIR)/houdiniEngine-maya$(MAYA_VERSION)
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
all: $(DST_MODULE) $(DST_PLUG_IN) $(DST_SCRIPTS)
endif

$(DST_MODULE):
	echo "+ MAYAVERSION:$(MAYA_VERSION) houdiniEngine 1.0 maya$(MAYA_VERSION)" > $(@)

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

.PHONY: clean
clean:
	rm -f $(DST_MODULE) $(DST_PLUG_IN) $(DST_SCRIPTS)
ifeq ($(OS), Cygwin)
	rm -f $(DST_PLUG_IN:%.$(SOSUFFIX)=%.lib) $(DST_PLUG_IN:%.$(SOSUFFIX)=%.exp)
endif
	rm -f $(OBJFILES)
	rm -f $(DEPFILES)
