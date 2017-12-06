#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <omp.h>
#include <mpi.h>
#include <time.h>

#define chunk 4
#define NUM_PI 3
#defin NUM_THREADS 4
#define MASTER 0

void encrypt (uint32_t* v, uint32_t* k);
void decrypt (uint32_t* v, uint32_t* k);
void mpEncrypt(uint32_t *text, uint32_t *key, int size);
void mpDecrypt(uint32_t *text, uint32_t *key, int size);
void plainEncrypt(uint32_t *text, uint32_t *key, int size);
void plainDecrypt(uint32_t *text, uint32_t *key, int size);
int verify(uint32_t *text, uint32_t *text_gold, int size);

int size;
int rank;

int main(int argc, char** argv) {

	// Determine encryption or decryption
	char* mode = argv[1];

	// Open files
	FILE *ifp, *ofp;
	int size_bytes;
	
	if (mode[0] == "-d"){ //decryption mode
		ifp = fopen("pt.txt", "r");
		ofp = fopen("ct.txt", "w");
	} else {//encryption
		ifp = fopen("ct.txt", "r");
		ofp = fopen("pt.txt", "w");
	}
	if (ifp == NULL) { 
		fprintf(stderr, "Can't open file\n");
	}
	if (ofp == NULL) {
		fprintf(stderr, "Can't open file\n");
	}
	fseek(ifp, 0L, SEEK_END);	
	size_bytes = ftell(ifp); 
	rewind(ifp);

	// Adjust size for padding purposes
	remainder = size_bytes % (NUM_PI*4);
	if (remainder != 0){
		size_bytes = size_bytes + (NUM_PI*4 - remainder)
	}
	//size in integer #'s
	int size32 = size_bytes/4

	// Setup OpemMPI
	//MPI_Init(&argc, &argv);
	//MPI_Comm_size(MPI_COMM_WORLD, &size);
	//MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	
	// Variables
	double timeStart, timeEnd;
	int text_size;
	int size_per_proc;

	// Allocate plaintext and key
	int i;
	uint32_t key[4] = {1, 2, 3, 4};
	uint32_t* text = calloc(size32, sizeof(uint32_t));
	uint32_t* text_decrypted = calloc(size32, sizeof(uint32_t));
	uint32_t* text_encrypted = calloc(size32, sizeof(uint32_t));
	uint32_t* text_sub = malloc(sizeof(uint32_t) * size32/NUM_PI);
	uint32_t* text_gold = calloc(size32, sizeof(uint32_t));

	// Read text into array
	fread(text, sizeof(int32_t), size, ifp);
	
	if(rank == MASTER){
		//Load keys and text into master thread
		timeStart = MPI_Wtime();
		text_size = size_bytes/4;
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

	if (mode[0] == "-d"){ 
		mpDecrypt(text_sub, key, size_per_proc);
		//plainDecrypt(text_sub, key, size_per_proc);
	} else {
		mpEncrypt(text_sub, key, size_per_proc);
		//plainEncrypt(text_sub, key, size_per_proc);
	}
	
	// Gather results
	MPI_Gather(text_sub, size_per_proc, MPI_UNSIGNED, text_decrypted, size_per_proc, MPI_UNSIGNED,
				MASTER, MPI_COMM_WORLD);
	
	//MPI Barrier for Synchronisation
	MPI_Barrier(MPI_COMM_WORLD);

	// Stop timer and verify
	if (rank == MASTER){
		timeEnd = MPI_Wtime();
		printf("Time Elapsed: %f\n", (timeEnd-timeStart) );
		if (verify(text_decrypted, text_gold) == 0){
			printf("Incorrect plaintext\n");
		} else {
			printf("Correct plaintext\n");
		}
	}

	MPI_Barrier(MPI_COMM_WORLD);
	return 0;

}

int verify(uint32_t *text, uint32_t *text_gold, int size){
	int i;
	int result = 1;
	for (i = 0; i < size; i += 2){
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
