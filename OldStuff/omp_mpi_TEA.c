#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <omp.h>
#include <mpi.h>
#include <time.h>

#define TEXT_BYTES 24000000
#define chunk 4
#define NUM_PI 3
#define MASTER 0

void encrypt (uint32_t* v, uint32_t* k);
void decrypt (uint32_t* v, uint32_t* k);
void mpEncrypt(uint32_t *text, uint32_t *key, int size);
void mpDecrypt(uint32_t *text, uint32_t *key, int size);
void plainEncrypt(uint32_t *text, uint32_t *key, int size);
void plainDecrypt(uint32_t *text, uint32_t *key, int size);
int verify(uint32_t *text, uint32_t *text_gold, int size);

int main(int argc, char** argv) {
	
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
	uint32_t* text_sub = malloc(sizeof(uint32_t) * TEXT_BYTES/4/NUM_PI);
	uint32_t* text_gold = malloc(sizeof(uint32_t) * TEXT_BYTES/4);
	for (i = 0; i < TEXT_BYTES/4; i++){
		text[i] = i%128;
		text_gold[i] = i%128;
	}
	
	if(rank == MASTER){
		//Load keys and text into master thread
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
		
	//Scatter dimensions for input image from Master process
	MPI_Scatter(text, size_per_proc, MPI_UNSIGNED, text_sub, size_per_proc,
				MPI_UNSIGNED, MASTER, MPI_COMM_WORLD);	

	//plainEncrypt(text_sub, key, size_per_proc);
	mpEncrypt(text_sub, key, size_per_proc);
	//plainDecrypt(text_sub, key, size_per_proc);
	mpDecrypt(text_sub, key, size_per_proc);
	
	// Gather results
	MPI_Gather(text_sub, size_per_proc, MPI_UNSIGNED, text_decrypted, size_per_proc, MPI_UNSIGNED,
				MASTER, MPI_COMM_WORLD);
	
	//MPI Barrier for Synchronisation
	//MPI_Barrier(MPI_COMM_WORLD);

	// Stop timer and verify
	if (rank == MASTER){
		timeEnd = MPI_Wtime();
		printf("Time Elapsed: %f\n", (timeEnd-timeStart) );
		if (verify(text_decrypted, text_gold, text_size) == 0){
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

void encrypt (uint32_t* v, uint32_t* k) {
    uint32_t v0=v[0], v1=v[1], sum=0, i;           /* set up */
    uint32_t delta=0x9e3779b9;                     /* a key schedule constant */
    uint32_t k0=k[0], k1=k[1], k2=k[2], k3=k[3];   /* cache key */
    for (i=0; i < 32; i++) {                       /* basic cycle start */
        sum += delta;
        v0 += ((v1<<4) + k0) ^ (v1 + sum) ^ ((v1>>5) + k1);
        v1 += ((v0<<4) + k2) ^ (v0 + sum) ^ ((v0>>5) + k3);
    }                                              /* end cycle */
    v[0]=v0; v[1]=v1;
}

void decrypt (uint32_t* v, uint32_t* k) {
    uint32_t v0=v[0], v1=v[1], sum=0xC6EF3720, i;  /* set up */
    uint32_t delta=0x9e3779b9;                     /* a key schedule constant */
    uint32_t k0=k[0], k1=k[1], k2=k[2], k3=k[3];   /* cache key */
    for (i=0; i<32; i++) {                         /* basic cycle start */
        v1 -= ((v0<<4) + k2) ^ (v0 + sum) ^ ((v0>>5) + k3);
        v0 -= ((v1<<4) + k0) ^ (v1 + sum) ^ ((v1>>5) + k1);
        sum -= delta;
    }                                              /* end cycle */
    v[0]=v0; v[1]=v1;
}
