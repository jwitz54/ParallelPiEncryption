tea: TEA.o
	g++ -o TEA TEA.o -fopenmp

mpi: MPITest.c
	mpicc -o MPITest MPITest.c
