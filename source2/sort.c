#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/wait.h>
#include <sys/types.h>

int main(int argc, char *argv[]){
	char strArr[20][256];
	char buf[256];
	int index = 0;
	int n = 0;
	bool isThisOptNotNull[3];
	char transMethod[256];
	char fileName[256];
	char fileSize[256];
	char *tempStr;
	extern char *optarg;
	extern int optint;
	int fd[2];
	pid_t pid;
	
	while((n = getopt(argc, argv, "t:f:s:")) != -1){
		switch (n) {
			case 't':
				strcpy(transMethod, optarg);
				isThisOptNotNull[0] = true;
				break;
			case 'f':
				strcpy(fileName, optarg);
				isThisOptNotNull[1] = true;
				break;
			case 's':
				strcpy(fileSize, optarg);
				isThisOptNotNull[2] = true;
				break;
		}
	}

	if(isThisOptNotNull[0] == false){
		strcpy(transMethod, "r.map");
	}
	if(isThisOptNotNull[1] == false){
		tempStr = getenv("SORT_COMM");
		if(tempStr == NULL){
			strcpy(fileName, "data.mmap");
		}
		else{
			strcpy(fileName, tempStr);
		}
	}
	if(isThisOptNotNull[2] == false){
		tempStr = getenv("SORT_COMM_BUF");
		if(tempStr == NULL){
			strcpy(fileSize, "4096");
		}
		else{
			strcpy(fileSize, tempStr);
		}
	}

	while(1){
		n = read(0, buf, 255);
		if(n == -1){
			perror("read error!");
		}

		buf[n-1] = '\0';
		if(n == 0){
			continue;
		}
		else if(strcmp(buf, "%sort") == 0){
			break;
		}
		else{
			buf[n-1] = '\n';
			buf[n] = '\0';
			strcpy(strArr[index++], buf);	
		}
	}
	buf[n-1] = '\n';
	buf[n] = '\0';
	strcpy(strArr[index++], buf);

	if(pipe(fd) == -1){
		perror("pipe");
		exit(1);
	}

	switch(pid = fork()){
		case -1:
			perror("fork");
			exit(1);
			break;
		case 0:
			close(fd[1]);
			if(fd[0] != 0){
				dup2(fd[0], 0);
				close(fd[0]);
			}
			execlp("./client.mmap", "./client.mmap", fileName, fileSize, (char *)NULL);
			exit(1);
			break;
		default:
			close(fd[0]);
			for(int i=0; i<index; i++){
				write(fd[1], strArr[i], strlen(strArr[i]));
			}
			wait(NULL);
			break;
	}

	return 0;
}
