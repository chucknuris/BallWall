#--------------------------------------------------------
CMP      = g++
SOURCE   = ballwall.cpp loadCompileShaders.cpp camera.cpp
OBJ      = $(SOURCE:.cpp=.o)
LIBS     = -lGL -lGLU -lGLEW -lglut -lm
CXXINCS  = 
BIN      = ballwall
OPTIM		 = -Ofast -Wall -Wextra -funroll-loops
CXXFLAGS = $(CXXINCS) $(OPTIM)
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
