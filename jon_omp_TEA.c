#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <omp.h>
#include <time.h>

//test
#define TEXT_BYTES 2400000
#define chunk 4
#define MASTER 0
#define NUM_ROUNDS 8
void encrypt (uint32_t* pt, uint32_t* key); 
void decrypt (uint32_t* ct, uint32_t* key);
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
	for(int i=0; i<NUM_ROUNDS; i++){
		plainEncrypt(text, key);
	}
	printf("Time to encrypt: %f\n", (omp_get_wtime() - start_time_standard) );
	
	for(int i=0; i<NUM_ROUNDS; i++){
		plainDecrypt(text, key);
	}
	if (verify(text, text_gold) == 0){
		printf("Incorrect plaintext\n");
	} else {
		printf("Correct plaintext\n");
	}
	
	printf("OpenMP Implementation\n");
	// Perform and time encryption
	start_time_omp = omp_get_wtime();
	//Perform NUM_ROUNDS of encryption
	for(int i=0; i<NUM_ROUNDS; i++){
		mpEncrypt(text, key);
	}
	printf("Time to encrypt: %f\n", (omp_get_wtime() - start_time_omp) );
	for(int i=0; i<NUM_ROUNDS; i++){
		mpDecrypt(text, key);
	}
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
