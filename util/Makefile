IDENTICALINC_DIR=/proj/identical/src/include
IDENTICALLIB_DIR=/proj/identical/lib

SDL_CFLAGS := $(shell sdl-config --cflags)
SDL_LDFLAGS := $(shell sdl-config --libs)

BTBUILDERINC_DIR=../src
BTBUILDEROBJ_DIR=../src

CXX = g++
CXXFLAGS = -g -frtti -DSDLLIB -I$(IDENTICALINC_DIR) -I$(BTBUILDEROBJ_DIR) $(SDL_CFLAGS) -I.
LIBS = -L$(IDENTICALLIB_DIR) -lidentical-sdl -ldl $(SDL_LDFLAGS)

PCX2PIAC_OBJ = pcx2piac.o compressor.o file.o
PCX2PIAC_EXE = pcx2piac

PIAC2PCX_OBJ = piac2pcx.o compressor.o file.o
PIAC2PCX_EXE = piac2pcx

DECOMP_OBJ = decomp.o compressor.o file.o
DECOMP_EXE = decomp

all: $(PCX2PIAC_EXE) $(PIAC2PCX_EXE) $(DECOMP_EXE)

$(PCX2PIAC_EXE): $(PCX2PIAC_OBJ)
	$(CXX) -o $(PCX2PIAC_EXE) $(PCX2PIAC_OBJ) $(LIBS)

$(PIAC2PCX_EXE): $(PIAC2PCX_OBJ)
	$(CXX) -o $(PIAC2PCX_EXE) $(PIAC2PCX_OBJ) $(LIBS)

$(DECOMP_EXE): $(DECOMP_OBJ)
	$(CXX) -o $(DECOMP_EXE) $(DECOMP_OBJ) $(LIBS)

%.o: %.C
	$(CXX) $(CXXFLAGS) -c -o $@ $<

%.o: %.c
	$(CXX) $(CXXFLAGS) -c -o $@ $<

clean:
	rm -f $(PCX2PIAC_OBJ) $(PCX2PIAC_EXE) $(DECOMP_OBJ) $(DECOMP_EXE)
	rm -f $(PIAC2PCX_OBJ) $(PIAC2PCX_EXE)

