SRC=src/glpainter.cpp src/main.cpp src/image.cpp
PROG=factor_trees
INC=-Isrc
LIBS=-lglut -lGL -lGLU
CC=g++
FLAGS=-O3

all: $(PROG)

$(PROG): $(SRC)
	$(CC) $(FLAGS) $(SRC) -o $(PROG) $(LIBS) $(INC)

clean:
	rm -f $(PROG)

