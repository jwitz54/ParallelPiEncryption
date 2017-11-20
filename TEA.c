#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <omp.h>
#include <mpi.h>
#include <time.h>

#define TEXT_BYTES 512

void encrypt (uint32_t* v, uint32_t* k);
void decrypt (uint32_t* v, uint32_t* k);
void mpEncrypt(uint32_t *text, uint32_t *key);
void mpDecrypt(uint32_t *text, uint32_t *key);
void plainEncrypt(uint32_t *text, uint32_t *key);
void plainDecrypt(uint32_t *text, uint32_t *key);
int verify(uint32_t *text, uint32_t *text_gold);

int main() {
	
	// Setup OpemMPI
	MPI_Init(NULL, NULL);

	int world_size;
	MPI_Comm_size(MPI_COMM_WORLD, &world_size);

	int world_rank;
	MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);


	// Set up key and plaintext block
	int i;
	uint32_t key[4] = {1, 1, 1, 1};
	uint32_t text[TEXT_BYTES/4];
	uint32_t text_gold[TEXT_BYTES/4];
	for (i = 0; i < TEXT_BYTES/4; i++){
		text[i] = 4;
		text_gold[i] = 4;
	}	

	// --------------OPENMP IMPLEMENTATION------------- 
	// Setup timing
	struct timeval start, end;
	long usecs;
	int tid;
	printf("OpenMP Implementation\n");

	// Perform and time encryption
	gettimeofday(&start, NULL);
	plainEncrypt(text, key);
	plainDecrypt(text, key);
	gettimeofday(&end, NULL);

	// Calculate time and verify
	usecs = end.tv_usec - start.tv_usec;
	printf("Time to encrypt/decrypt: %f\n", (float)usecs);
	if (verify(text, text_gold) == 0){
		printf("Incorrect plaintext\n");
	} else {
		printf("Correct plaintext\n");
	}

	return 0;

}

int verify(uint32_t *text, uint32_t *text_gold){
	int i;
	int result = 1;
	for (i = 0; i < TEXT_BYTES/4/2; i += 2){
		if (text[i] != text_gold[i]){
			printf("index: %d, text: %d, gold: %d", i, text[i], text_gold[i]);
			result = 0;
			break;
		}	
	}
	return result;
}

void plainEncrypt(uint32_t *text, uint32_t *key){
	int i;
	for (i = 0; i < TEXT_BYTES/4/2; i += 2){
		encrypt (&text[i], key);
	}
}

void plainDecrypt(uint32_t *text, uint32_t *key){
	int i;
	for (i = 0; i < TEXT_BYTES/4/2; i += 2){
		decrypt (&text[i], key);
	}
}

void mpEncrypt(uint32_t *text, uint32_t *key){
	int i;
	#pragma omp parallel for default(shared) private(i) schedule(dynamic, chunk) num_threads(4)
	for (i = 0; i < TEXT_BYTES/4/2; i += 2){
		encrypt (&text[i], key);
	}
}

void mpDecrypt(uint32_t *text, uint32_t *key){
	int i;
	#pragma omp parallel for default(shared) private(i) schedule(dynamic, chunk) num_threads(4)
	for (i = 0; i < TEXT_BYTES/4/2; i += 2){
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
