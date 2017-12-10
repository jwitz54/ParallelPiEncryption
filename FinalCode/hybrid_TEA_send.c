#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <omp.h>
#include <mpi.h>
#include <time.h>
#include <string.h>

//#define TEXT_BYTES 2400000
#define chunk 4
#define NUM_PI 3
#define MASTER 0
//#define NUM_ROUNDS 8

void encrypt (uint32_t* pt, uint32_t* key); 
void decrypt (uint32_t* ct, uint32_t* key);
void mpEncrypt(uint32_t *text, uint32_t *key, int size);
void mpDecrypt(uint32_t *text, uint32_t *key, int size);
void plainEncrypt(uint32_t *text, uint32_t *key, int size);
void plainDecrypt(uint32_t *text, uint32_t *key, int size);
int verify(uint32_t *text, uint32_t *text_gold, int size);

int main(int argc, char** argv) {
	
	int TEXT_BYTES = atoi(argv[1]);
	int NUM_ROUNDS = atoi(argv[2]);

	// Setup OpemMPI
	MPI_Init(&argc, &argv);
	int size;
	int rank;
	MPI_Comm_size(MPI_COMM_WORLD, &size);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	
	// Variables
	double timeStart, timeEnd;
	int text_size;
	int size_per_proc;

	// Fill plaintext and key
	int i;
	uint32_t key[4] = {1, 2, 3, 4};
	//uint32_t text[TEXT_BYTES/4];
	//uint32_t text_decrypted[TEXT_BYTES/4];
	//uint32_t text_sub[TEXT_BYTES/4/NUM_PI];
	//uint32_t text_gold[TEXT_BYTES/4];
	uint32_t* text = malloc(sizeof(uint32_t) * TEXT_BYTES/4);
	uint32_t* text_decrypted = malloc(sizeof(uint32_t) * TEXT_BYTES/4);
	uint32_t* text_encrypted = malloc(sizeof(uint32_t) * TEXT_BYTES/4);
	uint32_t* text_sub = malloc(sizeof(uint32_t) * TEXT_BYTES/4/NUM_PI);
	uint32_t* text_gold = malloc(sizeof(uint32_t) * TEXT_BYTES/4);
	for (i = 0; i < TEXT_BYTES/4; i++){
		text[i] = i%128;
		text_gold[i] = i%128;
	}
	
	if(rank == MASTER){
		// Start time and calculate sizes
		timeStart = MPI_Wtime();
		text_size = TEXT_BYTES/4;
		size_per_proc = text_size/NUM_PI;
	}
	
	//Broadcast Message size and key to all nodes
	MPI_Bcast(key, 4, MPI_UNSIGNED, MASTER, MPI_COMM_WORLD);
	MPI_Bcast(&text_size, 1, MPI_INT, MASTER, MPI_COMM_WORLD);
	MPI_Bcast(&size_per_proc, 1, MPI_INT, MASTER, MPI_COMM_WORLD);
	
	//MPI Barrier for Synchronisation
	MPI_Barrier(MPI_COMM_WORLD);
	
	// Send out data from master and receive from nodes
	if(rank == MASTER){ 
		MPI_Send( (text+size_per_proc), size_per_proc, MPI_UNSIGNED, 1, 0 , MPI_COMM_WORLD );
		MPI_Send( (text+2*size_per_proc), size_per_proc, MPI_UNSIGNED, 2, 0, MPI_COMM_WORLD );
		memcpy(text_sub, text, size_per_proc*4);
	}	
	if(rank == 1 || rank == 2){
		MPI_Recv( text_sub, size_per_proc, MPI_UNSIGNED, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
	}
	
	//Scatter dimensions for input image from Master process
	//MPI_Scatter(text, size_per_proc, MPI_UNSIGNED, text_sub, size_per_proc,
	//			MPI_UNSIGNED, MASTER, MPI_COMM_WORLD);	

	// Encrypt file
	for(i=0; i<NUM_ROUNDS; i++){
		mpEncrypt(text_sub, key, size_per_proc);
	}
	
	// Send file back to master
	if(rank!=MASTER){
		MPI_Send(text_sub, size_per_proc, MPI_UNSIGNED, 0, 0, MPI_COMM_WORLD);
	}
	
	// Receive and concatenate files
	if(rank == MASTER){
		memcpy(text_encrypted, text_sub, size_per_proc*4);
		MPI_Recv( (text_encrypted + size_per_proc), size_per_proc, MPI_UNSIGNED, 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		MPI_Recv( (text_encrypted + 2*size_per_proc), size_per_proc, MPI_UNSIGNED, 2, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
	}
	// Gather results
	//MPI_Gather(text_sub, size_per_proc, MPI_UNSIGNED, text_decrypted, size_per_proc, MPI_UNSIGNED,
	//			MASTER, MPI_COMM_WORLD);
	
	//MPI Barrier for Synchronisation
	//MPI_Barrier(MPI_COMM_WORLD);

	// Stop timer and verify
	if (rank == MASTER){
		timeEnd = MPI_Wtime();
		printf("Time Elapsed: %f\n", (timeEnd-timeStart) );
		for(i=0; i<NUM_ROUNDS; i++){
			mpDecrypt(text_encrypted, key, TEXT_BYTES/4);
		}
		if (verify(text_encrypted, text_gold, text_size) == 0){
			printf("Incorrect plaintext\n");
		} else {
			printf("Correct plaintext\n");
		}
	}

	MPI_Barrier(MPI_COMM_WORLD);
	MPI_Finalize();
	return 0;

}

int verify(uint32_t *text, uint32_t *text_gold, int size){
	int i;
	int result = 1;
	for (i = 0; i < size; i++){
		//printf("index: %d, text: %d, gold: %d\n", i, text[i], text_gold[i]);
		if (text[i] != text_gold[i]){
			printf("index: %d, text: %d, gold: %d\n", i, text[i], text_gold[i]);
			result = 0;
			break;
		}	
	}
	return result;
}

void plainEncrypt(uint32_t *text, uint32_t *key, int size){
	int i;
	for (i = 0; i < size; i+=2){
		encrypt (&text[i], key);
	}
}

void plainDecrypt(uint32_t *text, uint32_t *key, int size){
	int i;
	for (i = 0; i < size; i+=2){
		decrypt (&text[i], key);
	}
}

void mpEncrypt(uint32_t *text, uint32_t *key, int size){
	//omp_set_num_threads(4);
	int i;
	#pragma omp parallel for default(shared) private(i) schedule(dynamic, chunk) num_threads(4)
	for (i = 0; i < size; i += 2){
		//~ if (omp_get_thread_num() == 0){
			//~ printf("num threads:%d\n", omp_get_num_threads());
		//~ }
		encrypt (&text[i], key);
	}
}

void mpDecrypt(uint32_t *text, uint32_t *key, int size){
	//omp_set_num_threads(4);
	int i;
	#pragma omp parallel for default(shared) private(i) schedule(dynamic, chunk) num_threads(4)
	for (i = 0; i < size; i += 2){
		decrypt (&text[i], key);
	}
}

void encrypt (uint32_t* pt, uint32_t* key) {
	uint32_t lblock, rblock, sum;	
	uint32_t delta = 0x9e3779b9;

	sum = 0;
	lblock = pt[0];
	rblock = pt[1];
	int i;
	for (i = 0; i < 32; i++){
		sum += delta;
		lblock += (((rblock << 4) + key[0]) ^ (rblock + sum) ^ ((rblock >> 5) + key[1]));
		rblock += (((lblock << 4) + key[2]) ^ (lblock + sum) ^ ((lblock >> 5) + key[3])); 	
	}	 
	pt[0] = lblock;
	pt[1] = rblock; 
}

void decrypt (uint32_t* ct, uint32_t* key) {
	uint32_t lblock, rblock, sum;	
	uint32_t delta = 0x9e3779b9;

	sum = 0xC6EF3720;
	lblock = ct[0];
	rblock = ct[1];
	int i;
	for (i = 0; i < 32; i++){
		rblock -= (((lblock << 4) + key[2]) ^ (lblock + sum) ^ ((lblock >> 5) + key[3]));
		lblock -= (((rblock << 4) + key[0]) ^ (rblock + sum) ^ ((rblock >> 5) + key[1])); 	
		sum -= delta;
	}	 
	ct[0] = lblock;
	ct[1] = rblock; 
}
