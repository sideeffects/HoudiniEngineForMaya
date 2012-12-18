MAYA_DIR=/usr/autodesk/maya2013-x64
HOUDINI_INCLUDES=-I$(HI)/HAPI -I$(HI) -I$(HC)/include -I$(HC)/include/zlib -I$(HC)/include/OpenEXR

CC=g++
CFLAGS=-m64 -O0 -pthread -pipe -D_BOOL -DLINUX_64 -DREQUIRE_IOSTREAM -fPIC -Wno-deprecated -fno-gnu-keywords
DEBUGFLAGS=-g -gstabs+
LDFLAGS=-Wl,-Bsymbolic -Wl,-rpath,$(HDSO)
INCLUDES=-I. -I.. -I$(MAYA_DIR)/include -I/usr/X11R6/include $(HOUDINI_INCLUDES)
LIBS=-L$(MAYA_DIR)/lib -lOpenMaya -lFoundation -L$(HDSO) `python2.6 $(SHM)/list_link_libs.py HAPI`

all: asset.so

asset.so: assetNode.o assetCommand.o assetManager.o asset.o object.o util.o common.o geometryObject.o instancerObject.o assetNodeMonitor.o geometryPart.o plugin.o
	$(CC) -shared $(CFLAGS) $(LDFLAGS) -o asset.so assetNode.o assetCommand.o assetManager.o asset.o object.o util.o common.o \
	    geometryObject.o instancerObject.o assetNodeMonitor.o geometryPart.o plugin.o $(LIBS)

assetNode.o: assetNode.C assetNode.h
	$(CC) -c $(CFLAGS) $(DEBUGFLAGS) $(INCLUDES) assetNode.C

assetCommand.o: assetCommand.C assetCommand.h
	$(CC) -c $(CFLAGS) $(DEBUGFLAGS) $(INCLUDES) assetCommand.C

assetManager.o: assetManager.C assetManager.h
	$(CC) -c $(CFLAGS) $(DEBUGFLAGS) $(INCLUDES) assetManager.C

asset.o: asset.C asset.h
	$(CC) -c $(CFLAGS) $(DEBUGFLAGS) $(INCLUDES) asset.C

object.o: object.C object.h
	$(CC) -c $(CFLAGS) $(DEBUGFLAGS) $(INCLUDES) object.C

util.o: util.C util.h
	$(CC) -c $(CFLAGS) $(DEBUGFLAGS) $(INCLUDES) util.C

common.o: common.C common.h
	$(CC) -c $(CFLAGS) $(DEBUGFLAGS) $(INCLUDES) common.C

geometryObject.o: geometryObject.C geometryObject.h
	$(CC) -c $(CFLAGS) $(DEBUGFLAGS) $(INCLUDES) geometryObject.C

instancerObject.o: instancerObject.C instancerObject.h
	$(CC) -c $(CFLAGS) $(DEBUGFLAGS) $(INCLUDES) instancerObject.C

assetNodeMonitor.o: assetNodeMonitor.C assetNodeMonitor.h
	$(CC) -c $(CFLAGS) $(DEBUGFLAGS) $(INCLUDES) assetNodeMonitor.C

geometryPart.o: geometryPart.C geometryPart.h
	$(CC) -c $(CFLAGS) $(DEBUGFLAGS) $(INCLUDES) geometryPart.C

plugin.o: plugin.C
	$(CC) -c $(CFLAGS) $(DEBUGFLAGS) $(INCLUDES) plugin.C


clean:
	rm -f *.o
	rm -f *.so
	rm -f .*.swp
	rm -f *.gch
