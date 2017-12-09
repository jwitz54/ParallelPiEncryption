#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <omp.h>
#include <mpi.h>
#include <time.h>
#include <string.h>

#define chunk 1
#define NUM_PI 3
#define NUM_THREADS 4
#define MASTER 0

void encrypt (uint32_t* v, uint32_t* k);
void decrypt (uint32_t* v, uint32_t* k);
void mpEncrypt(uint32_t *text, uint32_t *key, int size);
void mpDecrypt(uint32_t *text, uint32_t *key, int size);
void plainEncrypt(uint32_t *text, uint32_t *key, int size);
void plainDecrypt(uint32_t *text, uint32_t *key, int size);
//int verify(uint32_t *text, uint32_t *text_gold, int size);

int size;
int rank;

int main(int argc, char** argv) {
	
	// Setup OpemMPI
	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &size);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	
	//printf("argc:%d\n", argc);

	// Determine encryption or decryption
	int mode = atoi(argv[1]);
	//int mode = 0;
	//Number of rounds to encrypt/decrypt
	int rounds = atoi(argv[2]);
	//int rounds = 1;
	//get input key
	char* input_key = argv[3];
	//char* input_key = "x!";
	char* input_file_path = argv[4];
	
	// Text size
	FILE *ifp, *ofp;
	int size32; 
	int text_size;
	int i;
	int size_per_proc;
	uint32_t* key = calloc(4, sizeof(uint32_t));
	uint32_t* text; 
	uint32_t* processed_text;
	//uint32_t* text_gold;
	
	if(rank == MASTER){
		//file path to encryption/decryption
		//~ char* input_file_path = argv[4];
		//char* input_file_path = "/tmp/file1.txt";
		
		//get length of given key
		int len_key = 0;
		while(input_key[len_key]!='!'){
			len_key++;
		}	

		// Open files
		int size_bytes;
		int original_size;
		
		if (mode == 0){ //encryption mode
			ifp = fopen(input_file_path, "r");
			ofp = fopen("/tmp/output_enc.txt", "w");
			
		}else if (mode == 1) {//decryption
			ifp = fopen(input_file_path, "r");
			ofp = fopen("/tmp/output_dec.txt", "w");
			
		}
		//CHECK IF INPUT FILE PATH IS VALID
		if (ifp == NULL) { 
			fprintf(stderr, "Can't open file in %d\n", rank);
		}
		if (ofp == NULL) {
			fprintf(stderr, "Can't open file out %d\n", rank);
		}
		
		//GET SIZE OF INPUT FILE
		fseek(ifp, 0L, SEEK_END);	
		size_bytes = ftell(ifp);
		original_size = size_bytes; 
		
		rewind(ifp);
			
		// Adjust size for padding purposes
		int remainder = size_bytes % (NUM_PI*4*2);
		if (remainder != 0){
			size_bytes = size_bytes + (NUM_PI*4*2 - remainder);
		}
		//size in integer #'s
		size32 = size_bytes/4;
		text = calloc(size32, sizeof(uint32_t));
		processed_text = calloc(size32, sizeof(uint32_t));
		//text_gold = calloc(size32, sizeof(uint32_t));
		
		//read in input file into text
		fread(text, sizeof(int32_t), original_size/4, ifp);
		memcpy(key, input_key, len_key);
		text_size = size_bytes/4;
		size_per_proc = text_size/NUM_PI;
	}
	// Allocate plaintext and key
	
	//Broadcast Message size and key to all nodes
	MPI_Bcast(key, 4, MPI_UNSIGNED, MASTER, MPI_COMM_WORLD);
	MPI_Bcast(&size32, 1, MPI_INT, MASTER, MPI_COMM_WORLD);
	MPI_Bcast(&text_size, 1, MPI_INT, MASTER, MPI_COMM_WORLD);
	MPI_Bcast(&size_per_proc, 1, MPI_INT, MASTER, MPI_COMM_WORLD);
	//MPI_Bcast(&rounds, 1, MPI_INT, MASTER, MPI_COMM_WORLD);
	//Allocate memory for text_sub
	uint32_t* text_sub = malloc(sizeof(uint32_t) * size32/NUM_PI);
	
	//MPI Barrier for Synchronisation
	MPI_Barrier(MPI_COMM_WORLD);
	
	//SENDING DATA OUT ON MASTER
	if(rank == MASTER){ 
		MPI_Send( (text+size_per_proc), size_per_proc, MPI_UNSIGNED, 1, 0 , MPI_COMM_WORLD );
		MPI_Send( (text+2*size_per_proc), size_per_proc, MPI_UNSIGNED, 2, 0, MPI_COMM_WORLD );
		memcpy(text_sub, text, size_per_proc*4);
	}
	//RECEIVING DATA ON SLAVES	
	if(rank == 1 || rank == 2){
		MPI_Recv( text_sub, size_per_proc, MPI_UNSIGNED, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);	
		
	}
		
	//Scatter dimensions for input image from Master process
	//MPI_Scatter(text, size_per_proc, MPI_UNSIGNED, text_sub, size_per_proc,
	//			MPI_UNSIGNED, MASTER, MPI_COMM_WORLD);	


	//DETERMINE WHETHER TO DECRYPT OR ENCRYPT BASED ON ARGUMENT GIVEN
	if (mode == 0){ 
		//printf("Encrypting File \n");
		
		for(i=0; i<rounds; i++){
			//plainEncrypt(text_sub, key, size_per_proc);
			//printf("textsub: %x key: %x rank: %d size: %d\n", text_sub, key, rank, size_per_proc);
			mpEncrypt(text_sub, key, size_per_proc);
			
		}
		
	}else if(mode == 1) {
		//printf("Decrypting File\n");
		
		for(i=0; i<rounds; i++){
			mpDecrypt(text_sub, key, size_per_proc);
		}
	}

	//SEND DATA BACK TO MASTER PI
	if(rank!=MASTER){
		MPI_Send(text_sub, size_per_proc, MPI_UNSIGNED, 0, 0, MPI_COMM_WORLD);
	}
	
	//PUT DATA BACK TOGETHER ON MASTER PI
	if(rank == MASTER){
		memcpy(processed_text, text_sub, size_per_proc*4);
		MPI_Recv( (processed_text + size_per_proc), size_per_proc, MPI_UNSIGNED, 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		MPI_Recv( (processed_text + 2*size_per_proc), size_per_proc, MPI_UNSIGNED, 2, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
	}	
	
	// Gather results
	//MPI_Gather(text_sub, size_per_proc, MPI_UNSIGNED, processed_text, size_per_proc, MPI_UNSIGNED,
	//			MASTER, MPI_COMM_WORLD);
	
	//MPI Barrier for Synchronisation
	MPI_Barrier(MPI_COMM_WORLD);

	// Stop timer and verify
	if (rank == MASTER){
		printf("Writing to output\n");
		//mpDecrypt(processed_text, key, size32);
		
		//write processed_text to output file
		fwrite(processed_text, sizeof(uint32_t), size32, ofp);
		fclose(ofp);	
		
	}
	
	MPI_Barrier(MPI_COMM_WORLD);
	MPI_Finalize();
	return 0;

}
/*
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
*/

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
	//printf("index ptr: %x\n", &i);
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
