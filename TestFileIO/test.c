#include <stdio.h>

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
	fread(text, sizeof(char), size_bytes, ifp);

	int i;
	for (i = 0; i < size_bytes; i++){
		printf("%c", text[i]);
	}	

	fwrite(text, sizeof(char), size_bytes, ofp);

}
