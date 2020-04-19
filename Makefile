ccsrc = $(wildcard src/*.cc) \
		$(wildcard libs/gmath/*.cc) \

csrc = $(wildcard src/*.c) \
	   $(wildcard libs/imago/*.c) \
	   $(wildcard libs/zlib/*.c) \
	   $(wildcard libs/libpng/*.c) \
	   $(wildcard libs/libjpeg/*.c) \
	   $(wildcard libs/drawtext/*.c) \
	   $(wildcard libs/ogg/*.c) \
	   $(wildcard libs/vorbis/*.c)

obj = $(ccsrc:.cc=.o) $(csrc:.c=.o)
dep = $(obj:.o=.d)
bin = game

warn = -pedantic -Wall -Wno-format-overflow
dbg = -g
opt = -O3 -ffast-math
inc = -Ilibs -Ilibs/imago -Ilibs/libpng -Ilibs/zlib -Ilibs/libjpeg \
	  -Ilibs/ogg -Ilibs/vorbis
def = -DNO_FREETYPE

CFLAGS = $(warn) -MMD $(dbg) $(opt) $(inc) $(def)
CXXFLAGS = $(warn) -MMD $(dbg) $(opt) $(inc) $(def)
LDFLAGS = $(libsys) $(libgl) -lpthread

sys ?= $(shell uname -s | sed 's/MINGW.*/mingw/')
ifeq ($(sys), mingw)
	obj = $(src:.c=.w32.o)
	dep = $(obj:.o=.d)
	bin = game.exe

	libgl = -lopengl32 -lglu32 -lglew32
	#TODO(zisis): link with assimp on Windoze
	libsys = -lmingw32 -mconsole

else
	libgl = -lGL -lGLU -lglut -lGLEW
	libsys = -ldl -lm -lassimp
endif

$(bin): $(obj)
	$(CXX) -o $@ $(obj) $(LDFLAGS)

-include $(dep)

.PHONY: clean
clean:
	rm -f $(obj) $(bin)

.PHONY: cleandep
cleandep:
	rm -f $(dep)
