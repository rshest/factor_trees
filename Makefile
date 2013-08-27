SRC=src/glpainter.cpp src/main.cpp src/image.cpp
PROG=factor_trees
INC=-Isrc
LIBS=-lglut -lGL -lGLU
CC=g++
FLAGS=-O3

ifeq ($(shell sh -c 'uname -s 2>/dev/null || echo not'),Darwin)
	# OS X, requires GLUT install
	LIBS=-framework GLUT -framework OPENGL
endif

all: $(PROG)

$(PROG): $(SRC)
	$(CC) $(FLAGS) $(SRC) -o $(PROG) $(LIBS) $(INC)

clean:
	rm -f $(PROG)

