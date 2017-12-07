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
	mpicc -fopenmp -o tea_hybrid omp_mpi_TEA.c 
	mpirun --hostfile machinefile -np 3 ./tea_hybrid
	
tea_hybrid_t: omp_mpi_TEA.c
	mpicc -o tea_hybrid omp_mpi_TEA.c
	mpirun --hostfile machinefile -np 3 ./tea_hybrid

tea_hybrid_snr: omp_mpi_TEA.c
	mpicc -fopenmp -o tea_hybrid_snr hybrid_TEA_send.c
	mpirun --hostfile machinefile -np 3 ./tea_hybrid_snr

tea_hybrid_snr_t: omp_mpi_TEA.c
	mpicc -fopenmp -o tea_hybrid_snr hybrid_TEA_send.c
	mpirun -np 3 ./tea_hybrid_snr
	
tea_hybrid_file: file_omp_mpi_TEA.c
	mpicc -fopenmp -o file_hybrid_tea file_omp_mpi_TEA.c
	mpirun --hostfile machinefile -np 3 ./file_hybrid_tea 0 1 1! /tmp/file1.txt
	#mpirun -np 3 ./file_hybrid_tea 0 1 1!
