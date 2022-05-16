#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <string.h>

int main(int argc, char *argv[]){
	int fd;
	int n;
	int i;
	int index = 0;
	int fileSize = atoi(argv[2]);
	char buf[256];
	char *ptr;
	caddr_t addr;

	
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
		n = read(0, buf, 256);
		if(n == -1){
			perror("read error!");
		}
		
		buf[n] = '\0';
		if(n == 0){
			continue;
		}
		else{
			i = 0;
			while(buf[i] != '\0'){
				addr[index++] = buf[i++];		
			}
			addr[index++] = '\0';	
		}

		if(strstr(buf, "%sort") != NULL){
			break;
		}
	}

	while(1){
		if(strstr(addr, "%sorted") != NULL){
			break;
		}
	}

	ptr = strtok(addr, "\n");
	while(ptr != NULL){
		if(strcmp(ptr, "%sorted") == 0){
			ptr = strtok(NULL, "\n");
			continue;
		}
		printf("%s\n", ptr);
		ptr = strtok(NULL, "\n");
	}
	
	msync(addr, (size_t)fileSize, MS_INVALIDATE);
	munmap(addr, (size_t)fileSize);
	return 0;

}
