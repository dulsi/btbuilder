SDL_CFLAGS := $(shell sdl-config --cflags)
SDL_LDFLAGS := $(shell sdl-config --libs)

CXX = g++
CC = gcc
CXXFLAGS = -g -DSDLLIB $(SDL_CFLAGS) -Isrc/
LIBS = $(SDL_LDFLAGS) -lSDL_image -lexpat -lSDL_ttf -lphysfs

BTBUILDER_OBJ = src/file.o src/dice.o src/monster.o src/spell.o src/item.o \
	src/map.o src/main.o src/btconst.o src/game.o src/psuedo3d.o \
	src/psuedo3dconfig.o src/job.o src/expatcpp.o src/xmlserializer.o \
	src/race.o src/display.o src/sdlextend.o src/pc.o src/module.o \
	src/physfsrwops.o src/compressor.o src/bitfield.o src/status.o \
	src/serialrect.o src/displayconfig.o src/skill.o src/statusbar.o \
	src/statusinfo.o src/valuelookup.o src/screenset.o src/combat.o \
	src/xpchart.o src/spelleffect.o src/combatant.o src/group.o \
	src/icon.o
BTBUILDER_EXE = btbuilder

all: $(BTBUILDER_EXE)

$(BTBUILDER_EXE): $(BTBUILDER_OBJ)
	$(CXX) -Wl,-E -o $(BTBUILDER_EXE) $(BTBUILDER_OBJ) $(LIBS)

%.o: %.C
	$(CXX) $(CXXFLAGS) -c -o $@ $<

%.o: %.c
	$(CC) $(CXXFLAGS) -c -o $@ $<

clean:
	rm -f $(BTBUILDER_OBJ) $(BTBUILDER_EXE)

# dependencies
file.o: file.h

