MAYA_DIR = /usr/autodesk/maya2014-x64
HOUDINI_INCLUDES = -I$(HI)/HAPI -I$(HI) -I$(HC)/include -I$(HC)/include/zlib -I$(HC)/include/OpenEXR

LIBNAME = asset

SOSUFFIX = so
SONAME = $(LIBNAME).$(SOSUFFIX)

CC = g++
CXX = g++

CPPFLAGS += -I. -I.. -I$(MAYA_DIR)/include -I/usr/X11R6/include $(HOUDINI_INCLUDES)
CPPFLAGS += -D_BOOL -DLINUX_64 -DREQUIRE_IOSTREAM

CXXFLAGS += -m64 -O0 -pthread -pipe -fPIC -Wno-deprecated -fno-gnu-keywords
CXXFLAGS += -g -gstabs+

LDFLAGS += -Wl,-Bsymbolic -Wl,-rpath,$(HDSO)
LDLIBS += -L$(MAYA_DIR)/lib -lOpenMaya -lFoundation -L$(HDSO) `python$(PYVER) $(SHM)/list_link_libs.py HAPI`

CXXFILES = \
	   assetNode.C \
	   assetCommand.C \
	   assetManager.C \
	   asset.C \
	   object.C \
	   util.C \
	   geometryObject.C \
	   instancerObject.C \
	   assetNodeMonitor.C \
	   geometryPart.C \
	   plugin.C

OBJ_DIR = .obj
DEP_DIR = .dep

OBJFILES = $(patsubst %.C, $(OBJ_DIR)/%.o, $(CXXFILES))

DEPFILES = $(patsubst %.C, $(DEP_DIR)/%.d, $(CXXFILES))

.PHONY: all
all: $(SONAME)

$(SONAME): $(OBJFILES)
	@mkdir -p $(dir $(@))
	$(CC) -shared $(CFLAGS) $(LDFLAGS) -o $(@) $(OBJFILES) $(LDLIBS)

$(OBJ_DIR)/%.o: %.C
	@mkdir -p $(dir $(@))
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -o $(@) -c $(<)

$(DEP_DIR)/%.d: %.C
	@echo generating dependency for $(<)
	@mkdir -p $(dir $(@))
	@$(CXX) -MM -MP -MT $(OBJ_DIR)/$(*).o $(CPPFLAGS) $(<) -o $(@)

ifneq ($(MAKECMDGOALS),clean)
-include $(DEPFILES)
endif

.PHONY: clean
clean:
	rm -f $(SONAME)
	rm -f $(OBJFILES)
	rm -f $(DEPFILES)
