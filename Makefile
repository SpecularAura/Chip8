CC = g++
CC_FLAGS = -g -std=c++17 -Wall -Wextra -Werror -Wno-error=unknown-pragmas -Wno-error=unused-variable

DEBUG=NODEBUG

IDIRSDL = ./SDL2
IDIR = ./dependencies/Headers

INCLUDEMAIN = -I$(IDIR) -I$(IDIRSDL)
INCLUDEDEP  = -I$(IDIR) -I$(IDIRSDL)

LIBDIR = ./SDL2/bin
LIBS = -L$(LIBDIR)

LIBLINK = -lmingw32 -lSDL2main -lSDL2 #-mwindows
#_DEPS = testinclude.h
_DEPS = $(wildcard $(IDIR)/*.h)
#DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))
DEPS = $(_DEPS)

SRCDIR = ./dependencies/Source_code
ODIR = ./dependencies/Objects
#_CPP = testinclude.cpp
_CPP = $(wildcard $(SRCDIR)/*.cpp)
#CPP = $(patsubst %,$(SRCDIR)/%,$(_CPP))
CPP = $(_CPP)

ifeq ($(DEBUG), NODEBUG)
CPP = $(_CPP) $(SRCDIR)/Instructions/ProdFunctions.cpp
else
CPP = $(_CPP) $(SRCDIR)/Instructions/DebugFunctions.cpp
endif

#_OBJ = $(patsubst %.cpp,%.o,$(CPP))
_OBJ = $(patsubst $(SRCDIR)/%.cpp,$(ODIR)/%.o,$(CPP))
OBJ = $(_OBJ)

EXECUTABLE = main

$(EXECUTABLE):$(OBJ)
	$(CC) $(CC_FLAGS) $@.cpp -D$(DEBUG) $(INCLUDEMAIN) $(LIBS) $(OBJ) -o $@ $(LIBLINK)

test: test.cpp $(OBJ)
	$(CC) $(CC_FLAGS) $@.cpp -D$(DEBUG) $(INCLUDEMAIN) $(LIBS) $(OBJ) -o $@ $(LIBLINK) 

testemu: testemu.cpp $(OBJ)
	$(CC) $(CC_FLAGSS) $@.cpp -D$(DEBUG) $(INCLUDEMAIN) $(LIBS) $(OBJ) -o $@ $(LIBLINK)

$(ODIR)/%.o:$(SRCDIR)/%.cpp $(DEPS) 
	$(CC) $(CC_FLAGS) -D$(DEBUG) -c $< $(INCLUDEDEP) -o $@

.PHONY: clean

clean:
	rm -r $(ODIR)/*.o *.exe
