#--------------------------------------------------------
CMP      = g++
SOURCE   = ballwall.cpp loadCompileShaders.cpp camera.cpp
OBJ      = $(SOURCE:.cpp=.o)
LIBS     = -static -lstdc++ -lfreeglut -lglew32 -lole32 -lglu32 -lglew32 -lopengl32 -luuid -lcomctl32 -lwsock32 -lgdi32 -lwinmm -lm -m64 -s
CXXINCS  = 
BIN      = ballwall
OPTIM		 = -Ofast -m64 -Wall -Wextra -funroll-loops
CXXFLAGS = $(CXXINCS) -DFREEGLUT_STATIC -DGLEW_STATIC $(OPTIM)
RM       = rm -f

.PHONY: all all-before all-after clean clean-custom

%.o: %.cpp
	$(CMP) $(CXXFLAGS) -c -o $@ $<

all:	all-before $(BIN) all-after

$(BIN): $(OBJ)
	#$(SOURCE)
	$(CMP) -o $(BIN) $(OBJ) $(LIBS)

clean: clean-custom
	$(RM) $(OBJ) $(BIN)
#--------------------------------------------------------
