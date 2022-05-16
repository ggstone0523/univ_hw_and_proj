#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <string.h>

void swap(char *str1, char *str2){
	char *temp = (char *)malloc(256 * sizeof(char));
	strcpy(temp, str1);
	strcpy(str1, str2);
	strcpy(str2, temp);
	free(temp);
}

int main(int argc, char *argv[]){
	int fd;
	int i;
	int j;
	int index = 0;
	int count = 0;
	int fileSize = atoi(argv[2]);
	char strArr[20][256];
	char *ptr;
	struct stat fsbuf;
	caddr_t addr;


	while(1){
		for(i=0; i<20; i++){
			for(j=0; j<256; j++){
				strArr[i][j] = '\0';
			}
		}
		index = 0;
		count = 0;

		if((fd = open(argv[1], O_RDWR | O_CREAT, 0666)) == -1){
			perror("open");
			exit(1);
		}

		if(ftruncate(fd, (off_t)fileSize) == -1){
			perror("ftruncate");
			exit(1);
		}

		addr = mmap(NULL, (size_t)fileSize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, (off_t)0);
		if(addr == MAP_FAILED){
			perror("mmap");
			exit(1);
		}

		close(fd);
		while(1){
			if(strstr(addr, "%sort") != NULL){
				break;
			}

		}
	
		ptr = strtok(addr, "\n");
		while(ptr != NULL){
			strcpy(strArr[index++], ptr);
			ptr = strtok(NULL, "\n");
		}
		strcpy(strArr[index-1], "%sorted");

		for(i=0; i<index-2; i++){
			for(j=0; j<index-i-2; j++){
				if(strlen(strArr[j]) > strlen(strArr[j+1])){
					swap(strArr[j], strArr[j+1]);
				}
				else if(strlen(strArr[j]) == strlen(strArr[j+1])){
					if(strcmp(strArr[j], strArr[j+1]) > 0){
						swap(strArr[j], strArr[j+1]);								     }
				}
				else{
					continue;
				}
			}
		}

		for(i=0; i<index; i++){
			for(j=0; j<strlen(strArr[i]); j++){
				addr[count++] = strArr[i][j];
			}
			addr[count++] = '\n';
		}
		addr[count++] = '\0';
		
		msync(addr, (size_t)fileSize, MS_SYNC);
		munmap(addr, (size_t)fileSize);
	}

	return 0;
}
