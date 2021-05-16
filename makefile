#--------------------------------------------------------
CMP      = g++
SOURCE   = ballwall.cpp loadCompileShaders.cpp camera.cpp
OBJ      = $(SOURCE:.cpp=.o)
LIBS     = -lm
CXXINCS  = 
BIN      = ballwall
OPTIM		 =  -O2
CXXFLAGS = -std=c++11 $(CXXINCS) $(OPTIM)
LDFLAGS  = -framework OpenGL -framework GLUT
RM       = rm -f

.PHONY: all all-before all-after clean clean-custom

%.o: %.cpp
	$(CMP) $(CXXFLAGS) -c -o $@ $<

all:	all-before $(BIN) all-after

$(BIN): $(OBJ)
	#$(SOURCE)
	$(CMP) -o $(BIN) $(OBJ) $(LIBS) $(LDFLAGS)

clean: clean-custom
	$(RM) $(OBJ) $(BIN)
#--------------------------------------------------------
