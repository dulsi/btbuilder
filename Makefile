SDL_CFLAGS := $(shell sdl-config --cflags)
SDL_LDFLAGS := $(shell sdl-config --libs)

CXX = g++
CC = gcc
CXXFLAGS = -g -DSDLLIB $(SDL_CFLAGS) -Isrc/ --std=c++11
LIBS = $(SDL_LDFLAGS) --std=c++11 -lSDL_image -lSDL_mixer -lexpat -lSDL_ttf -lphysfs \
	-lboost_filesystem -lboost_system -lpng

BTBUILDER_OBJ = src/file.o src/dice.o src/monster.o src/spell.o src/item.o \
	src/map.o src/main.o src/btconst.o src/game.o src/psuedo3d.o \
	src/psuedo3dconfig.o src/job.o src/expatcpp.o src/xmlserializer.o \
	src/race.o src/display.o src/sdlextend.o src/pc.o src/module.o \
	src/physfsrwops.o src/compressor.o src/bitfield.o src/status.o \
	src/serialrect.o src/displayconfig.o src/skill.o src/statusbar.o \
	src/statusinfo.o src/valuelookup.o src/screenset.o src/combat.o \
	src/xpchart.o src/spelleffect.o src/combatant.o src/group.o \
	src/icon.o src/song.o src/manifest.o src/chest.o src/mainscreen.o \
	src/shop.o src/effectgroup.o src/psuedo3dmap.o src/editor.o \
	src/serialeditor.o src/SDL_mng.o
BTBUILDER_EXE = btbuilder

ifndef prefix
	prefix=
endif
BINDIR=${prefix}/usr/bin
DATADIR=${prefix}/usr/share
BTBUILDERDIR=${DATADIR}/btbuilder

all: $(BTBUILDER_EXE)

$(BTBUILDER_EXE): $(BTBUILDER_OBJ)
	$(CXX) -o $(BTBUILDER_EXE) $(BTBUILDER_OBJ) $(LIBS)

%.o: %.C
	$(CXX) $(CXXFLAGS) -DBTBUILDERDIR=${BTBUILDERDIR} -c -o $@ $<

%.o: %.c
	$(CC) $(CXXFLAGS) -DBTBUILDERDIR=${BTBUILDERDIR} -c -o $@ $<

install: all
	if test ! -d $(BINDIR); then mkdir -p $(BINDIR); fi
	if test ! -d $(DATADIR)/btbuilder; then mkdir -p $(DATADIR)/btbuilder; fi
	if test ! -d $(DATADIR)/icons/hicolor; then mkdir -p $(DATADIR)/icons/hicolor; fi
	cp -R data image module music $(DATADIR)/btbuilder/
	cp -R icons/* $(DATADIR)/icons/hicolor/
	cp btbuilder $(BINDIR)
	desktop-file-install --dir=$(DATADIR)/applications btbuilder.desktop

clean:
	rm -f $(BTBUILDER_OBJ) $(BTBUILDER_EXE)

# dependencies
file.o: file.h

