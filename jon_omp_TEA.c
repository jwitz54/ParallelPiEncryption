#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <omp.h>
#include <time.h>

//test
#define TEXT_BYTES 240000000
#define chunk 4
#define MASTER 0

void encrypt (uint32_t* v, uint32_t* k);
void decrypt (uint32_t* v, uint32_t* k);
void mpEncrypt(uint32_t *text, uint32_t *key);
void mpDecrypt(uint32_t *text, uint32_t *key);
void plainEncrypt(uint32_t *text, uint32_t *key);
void plainDecrypt(uint32_t *text, uint32_t *key);
int verify(uint32_t *text, uint32_t *text_gold);

int main() {
		
	

	// Set up key and plaintext block
	int i;
	uint32_t key[4] = {1, 2, 3, 4};
	uint32_t* text = (uint32_t*) malloc(sizeof(uint32_t) * TEXT_BYTES/4);
	//uint32_t* text_decrypted = malloc(sizeof(uint32_t) * TEXT_BYTES/4);
	//uint32_t* text_sub = malloc(sizeof(uint32_t) * TEXT_BYTES/4/NUM_PI);
	uint32_t* text_gold = (uint32_t*) malloc(sizeof(uint32_t) * TEXT_BYTES/4);
	//uint32_t text[TEXT_BYTES/4];
	//uint32_t text_gold[TEXT_BYTES/4];
	for (i = 0; i < TEXT_BYTES/4; i++){
		text[i] = i%128;
		text_gold[i] = i%128;
	}	


	//MAIN THREAD

	// Setup timing
	struct timeval start, end;
	double start_time_omp, start_time_standard, time;
	long usecs;
	int tid;
	
	printf("Standard Implementation\n");
	start_time_standard = omp_get_wtime();
	plainEncrypt(text, key);
	
	printf("Time to encrypt: %f\n", (omp_get_wtime() - start_time_standard) );
	plainDecrypt(text, key);
	if (verify(text, text_gold) == 0){
		printf("Incorrect plaintext\n");
	} else {
		printf("Correct plaintext\n");
	}
	
	printf("OpenMP Implementation\n");
	// Perform and time encryption
	start_time_omp = omp_get_wtime();
	mpEncrypt(text, key);
	
	printf("Time to encrypt: %f\n", (omp_get_wtime() - start_time_omp) );
	

	// Calculate time and verify
	//usecs = end.tv_usec - start.tv_usec;
	mpDecrypt(text, key);
	if (verify(text, text_gold) == 0){
		printf("Incorrect plaintext\n");
	} else {
		printf("Correct plaintext\n");
	}
	//SLAVE THREADS	

	return 0;

}

int verify(uint32_t *text, uint32_t *text_gold){
	int i;
	int result = 1;
	for (i = 0; i < TEXT_BYTES/4; i++){
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
	for (i = 0; i < TEXT_BYTES/4; i += 2){
		encrypt (&text[i], key);
	}
}

void plainDecrypt(uint32_t *text, uint32_t *key){
	int i;
	for (i = 0; i < TEXT_BYTES/4; i += 2){
		decrypt (&text[i], key);
	}
}

void mpEncrypt(uint32_t *text, uint32_t *key){
	//omp_set_num_threads(4);
	int i;
	//#pragma omp parallel for private(i) 
	#pragma omp parallel for default(shared) private(i) schedule(dynamic, chunk) num_threads(4)
	for (i = 0; i < TEXT_BYTES/4; i += 2){
		encrypt (&text[i], key);
	}
	
}

void mpDecrypt(uint32_t *text, uint32_t *key){
	//omp_set_num_threads(4);
	int i;
	//#pragma omp parallel for private(i)  
	#pragma omp parallel for default(shared) private(i) schedule(dynamic, chunk) num_threads(4)
	for (i = 0; i < TEXT_BYTES/4; i += 2){
	
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
