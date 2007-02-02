IDENTICALINC_DIR=/proj/identical/src/include
IDENTICALLIB_DIR=/proj/identical/lib

SDL_CFLAGS := $(shell sdl-config --cflags)
SDL_LDFLAGS := $(shell sdl-config --libs)

CXX = g++
CXXFLAGS = -g -frtti -DSDLLIB -I$(IDENTICALINC_DIR) $(SDL_CFLAGS) -Isrc/
LIBS = -L$(IDENTICALLIB_DIR) -lidentical-sdl -ldl $(SDL_LDFLAGS) -lSDL_image \
	-lexpat

BTBUILDER_OBJ = src/file.o src/dice.o src/monster.o src/spell.o src/item.o \
	src/map.o src/main.o src/btconst.o src/game.o src/psuedo3d.o \
	src/psuedo3dconfig.o src/job.o src/expatcpp.o src/xmlserializer.o
BTBUILDER_EXE = btbuilder

all: $(BTBUILDER_EXE)

$(BTBUILDER_EXE): $(BTBUILDER_OBJ)
	$(CXX) -Wl,-E -o $(BTBUILDER_EXE) $(BTBUILDER_OBJ) $(LIBS)

%.o: %.C
	$(CXX) $(CXXFLAGS) -c -o $@ $<

clean:
	rm -f $(BTBUILDER_OBJ) $(BTBUILDER_EXE)

# dependencies
file.o: file.h

