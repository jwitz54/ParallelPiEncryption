tea: TEA.c
	mpicc -o TEA TEA.c 
	mpirun -np 4 ./TEA

mpi: MPITest.c
	mpicc -o MPITest MPITest.c -fopenmp
