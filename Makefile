tea: TEA.c
	mpicc -o TEA TEA.c 
	mpirun -np 4 ./TEA

jmpi: jon_mpi_TEA.c
	mpicc -o jon_mpi_TEA jon_mpi_TEA.c
	mpirun -np 3 ./jon_mpi_TEA

mpi: MPITest.c
	mpicc -o MPITest MPITest.c -fopenmp
	
tea_omp: jon_omp_TEA.c
	g++ -fopenmp -o tea_omp jon_omp_TEA.c

tea_mpi: jon_mpi_TEA.c
	mpicc -o tea_mpi jon_mpi_TEA.c
	
