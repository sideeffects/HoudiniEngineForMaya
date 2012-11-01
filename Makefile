MAYA_DIR=/usr/autodesk/maya2013-x64
HOUDINI_INCLUDES=-I$(HI)/HAPI -I$(HI) -I$(HC)/include -I$(HC)/include/zlib -I$(HC)/include/OpenEXR

CC=g++
CFLAGS=-m64 -O0 -pthread -pipe -D_BOOL -DLINUX_64 -DREQUIRE_IOSTREAM -fPIC -Wno-deprecated -fno-gnu-keywords
DEBUGFLAGS=-g -gstabs+
LDFLAGS=-Wl,-Bsymbolic -Wl,-rpath,$(HDSO)
INCLUDES=-I. -I.. -I$(MAYA_DIR)/include -I/usr/X11R6/include $(HOUDINI_INCLUDES)
LIBS=-L$(MAYA_DIR)/lib -lOpenMaya -lFoundation -L$(HDSO) `python2.6 $(SHM)/list_link_libs.py HAPI`

all: asset.so

asset.so: plugin.o asset.o object.o util.o common.o
	$(CC) -shared $(CFLAGS) $(LDFLAGS) -o asset.so plugin.o asset.o object.o util.o $(LIBS)

plugin.o: plugin.C plugin.h
	$(CC) -c $(CFLAGS) $(DEBUGFLAGS) $(INCLUDES) plugin.C plugin.h

asset.o: asset.C asset.h
	$(CC) -c $(CFLAGS) $(DEBUGFLAGS) $(INCLUDES) asset.C asset.h

object.o: object.C object.h
	$(CC) -c $(CFLAGS) $(DEBUGFLAGS) $(INCLUDES) object.C object.h

util.o: util.C util.h
	$(CC) -c $(CFLAGS) $(DEBUGFLAGS) $(INCLUDES) util.C util.h

common.o: common.C common.h
	$(CC) -c $(CFLAGS) $(DEBUGFLAGS) $(INCLUDES) common.C common.h


clean:
	rm -f *.o
	rm -f *.so
	rm -f *.swp
	rm -f *.gch
