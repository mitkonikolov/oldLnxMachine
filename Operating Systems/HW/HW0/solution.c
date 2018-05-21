#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>

int main() {
	int temp;
	char *file = "/proc/2144/maps";
	temp = open(file, O_RDONLY);
	if(temp==-1) {
		printf("Error is %d\n", errno);
	}
	else {
	        printf("%d\n", temp);
	        printf("%x\n", file);
		char init;
		char res[100000];
		//void *res1 = malloc(3000);
		size_t bytes = 3000;
		ssize_t readBytes;
		size_t totReadBytes=0;
		errno=0;
		do {
			// read the file into a buffer
			readBytes = read(temp, &res[totReadBytes], 1);
			//readBytes = read(temp, &res1, 1);
			totReadBytes = totReadBytes + 1;
		}while(readBytes>0);
		if (errno!=0) {
			printf("Error number is %d\n", errno);
		}
		else {
			if(close(temp)!=0) {
				printf("Problem closing the file");
			}
			else {
				printf("Read bytes are %d\n", totReadBytes);
				for(int i=0; i<totReadBytes; i++) {
					//printf("Char at %d  is %c\n", i, res[i]);
				}
				FILE *fptr;
				fptr = fopen("/home/mitko/CS/Operating Systems/HW/HW0/text1.txt", "w");
				fprintf(fptr, "%c", res[0]);
				
				int currByte = 0;
				while(res[currByte]!=' ') {
					fprintf(fptr, "%c", res[currByte]);
					printf("Char at %d  is %c\n", currByte, res[currByte]);
					currByte = currByte + 1;
				}
				currByte = currByte + 1;
				fprintf(fptr, ", isReadable=%s", res[currByte]=='r' ? "true" : "false");
                                printf("isReadable=%s\n", res[currByte]=='r' ? "true" : "false");
                                
				currByte = currByte + 1;
                                fprintf(fptr, ", isWritable=%s", res[currByte]=='w' ? "true" : "false");
                                printf("isWritable=%s\n", res[currByte]=='w' ? "true" : "false");

				currByte = currByte + 1;
                                fprintf(fptr, ", isExec=%s", res[currByte]=='x' ? "true" : "false");
                                printf("isExec=%s\n", res[currByte]=='x' ? "true" : "false");


			}
		}
	}
}


