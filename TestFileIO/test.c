#include <stdio.h>
#include <stdint.h>

// FROM http://www.cs.bu.edu/teaching/c/file-io/intro/

int main(int argc, char** argv) {

	FILE *ifp, *ofp;
	int size_bytes;
	
	ifp = fopen("pt.txt", "r");
	ofp = fopen("ct.txt", "w");

	if (ifp == NULL) {
		fprintf(stderr, "Can't open file\n");
		//exit(1);
	}
	if (ofp == NULL) {
		fprintf(stderr, "Can't open file\n");
		//exit(1);
	}

	fseek(ifp, 0L, SEEK_END);	
	size_bytes = ftell(ifp); 
	rewind(ifp);

	char* text = calloc(size_bytes, sizeof(char));
	int size_32 = (size_bytes + 4 - 1)/4;
	uint32_t* text32 = calloc(size_32, sizeof(uint32_t)); 
	fread(text, sizeof(char), size_bytes, ifp);
	rewind(ifp);
	fread(text32, sizeof(int32_t), size_32, ifp);
	char* text32_cptr = (char*)text32;

	int i;
	for (i = 0; i < size_bytes; i++){
		printf("%c", text[i]);
	}	
	//printf("\n");
	for (i = 0; i < size_32*sizeof(int32_t); i++){
		printf("%c", text32_cptr[i]);
	}	
	//printf("\n");
	//fwrite(text, sizeof(char), size_bytes, ofp);

}
