MAYA_DIR=/usr/autodesk/maya2013-x64
HOUDINI_DIR=/home/jhuang/dev/hfs
HOUDINI_INCLUDES=-I$(HOUDINI_DIR)/custom/houdini/include/HAPI -I$(HOUDINI_DIR)/custom/houdini/include -I$(HOUDINI_DIR)/custom/include -I$(HOUDINI_DIR)/custom/include/zlib -I$(HOUDINI_DIR)/custom/include/OpenEXR

CC=g++
CFLAGS=-m64 -O3 -pthread -pipe -D_BOOL -DLINUX_64 -DREQUIRE_IOSTREAM -fPIC -Wno-deprecated -fno-gnu-keywords
LDFLAGS=-Wl,-Bsymbolic -Wl,-rpath,/home/jhuang/dev/hfs/dsolib
INCLUDES=-I. -I.. -I$(MAYA_DIR)/include -I/usr/X11R6/include $(HOUDINI_INCLUDES)
LIBS=-L$(MAYA_DIR)/lib -lOpenMaya -lFoundation -L$(HOUDINI_DIR)/dsolib `python2.6 /home/jhuang/dev/src/houdini/make/list_link_libs.py HAPI`

all: asset.so

asset.so: asset.o
	$(CC) -shared $(CFLAGS) $(LDFLAGS) -o asset.so asset.o $(LIBS)

asset.o: asset.C
	$(CC) -c $(CFLAGS) $(INCLUDES) asset.C

clean:
	rm -f *.o
	rm -f *.so
	rm -f *.swp
