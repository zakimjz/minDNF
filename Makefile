PARAM=-O3 -Wno-deprecated
CC=g++
min-gen-DNF: random.o Clause.h DB.h DNF.h helper_funs.h Lattice_Node.h\
Random_Walk.h Set.h min-gen-DNF.cpp
	$(CC) $(PARAM) -o min-gen-DNF random.o min-gen-DNF.cpp

random.o: random.h random.cpp
	$(CC) $(PARAM) -c random.cpp random.h

clean:
	rm -f *.o
