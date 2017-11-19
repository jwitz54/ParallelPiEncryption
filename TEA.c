#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <omp.h>
#include <time.h>

#define TEXT_BYTES 16384

void encrypt (uint32_t* v, uint32_t* k);
void decrypt (uint32_t* v, uint32_t* k);

int main() {
	
	// Set up key and plaintext block
	int i;
	uint32_t key[4] = {1, 1, 1, 1};
	uint32_t text[TEXT_BYTES/4];
	for (i = 0; i < TEXT_BYTES/4; i++){
		text[i] = 4;
	}	

	// Encrypt and decrypt block
	printf("OpenMP Implementation\n");
	struct timeval start, end;
	long usecs;
	int tid;
	omp_set_num_threads(4);
	tid = omp_get_thread_num();
	if (tid == 0){
		printf("num threads: %i\n", omp_get_num_threads());
		printf("num cores: %i\n", omp_get_num_procs());
	}	

	gettimeofday(&start, NULL);
	{
		#pragma omp parallel for default(shared) private(i) schedule(dynamic, chunk) num_threads(4)
		for (i = 0; i < TEXT_BYTES/4/2; i += 2){
			encrypt (&text[i], key);
		}
		#pragma omp parallel for default(shared) private(i) schedule(dynamic, chunk) num_threads(4)
		for (i = 0; i < TEXT_BYTES/4/2; i += 2){
			decrypt (&text[i], key);
		}
	}
	gettimeofday(&end, NULL);
	
	usecs = end.tv_usec - start.tv_usec;

	// Verify block
	for (i = 0; i < TEXT_BYTES/4; i++){
		//printf("b%d: %d\n", i, text[i]);
	}	
	printf("Time to encrypt/decrypt: %f\n", (float)usecs);

	printf("Plain C Implementation\n");

	gettimeofday(&start, NULL);
	{
		for (i = 0; i < TEXT_BYTES/4/2; i += 2){
			encrypt (&text[i], key);
		}
		for (i = 0; i < TEXT_BYTES/4/2; i += 2){
			decrypt (&text[i], key);
		}
	}
	gettimeofday(&end, NULL);

	usecs = end.tv_usec - start.tv_usec;

	// Verify block
	for (i = 0; i < TEXT_BYTES/4; i++){
		//printf("b%d: %d\n", i, text[i]);
	}	
	printf("Time to encrypt/decrypt: %f\n", (float)usecs);

	return 0;

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
