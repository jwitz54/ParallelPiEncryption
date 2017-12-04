tea: TEA.c
	mpicc -o TEA TEA.c 
	mpirun -np 4 ./TEA

tea_mpi: jon_mpi_TEA.c
	mpicc -o tea_mpi jon_mpi_TEA.c
	mpirun --hostfile machinefile -np 3 ./tea_mpi

mpi: MPITest.c
	mpicc -o MPITest MPITest.c -fopenmp
	
tea_omp: jon_omp_TEA.c
	g++ -fopenmp -o tea_omp jon_omp_TEA.c
	./tea_omp

tea_hybrid: omp_mpi_TEA.c
	mpicc -o tea_hybrid omp_mpi_TEA.c
	mpirun --hostfile machinefile -np 3 ./tea_hybrid
	
