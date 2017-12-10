File overview for file server:
client_mod.py - Run on client to send files to encryption server
host_mod.py - Run on cluster to receive and encrypt files
file_omp_mpi_TEA.c - Compile on master and node Pi's to run parallel MPI

File overview for testing:
hybrid_tea_send.c - Used to measure performance and correctness of hybrid implementation
jon_omp_TEA.c - Used to measure performance and correctness of OpenMP and standard implementations 
