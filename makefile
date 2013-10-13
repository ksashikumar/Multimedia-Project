FLAGS = -O2 -I/usr/include/GL
LIBS = -lGL -lglut -lGL -lGLU -lglui -lX11 -lm  -lstdc++ -lSDL -lSDL_image -lSDL_mixer -lSDL_ttf

all: final

final: final.cpp
	gcc -g final.cpp -o Project $(FLAGS) $(LIBS)

