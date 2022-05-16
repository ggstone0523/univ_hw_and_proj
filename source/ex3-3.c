#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <stdbool.h>

int checkFileFifo(char *str, char *splitPara);

int main(int argc, char *argv[]){
	int fd, n;
	char buf[256];
	struct stat fsbuf;
	bool notFoundDataTxt = true;
	DIR *dp;
	struct dirent *dent;
	int kind;
	int PermCheck;
	int changeTime;
	int saveSize;
	int isThisFifo;
	char *fileStr;

	// 환경변수 COM_FILE의 값(파일명)을 fileStr변수에 저장	
	fileStr = getenv("COM_FILE");
	
	// 실행인자를 받아 각 인자에 알맞게 모드를 설정
	if(argc == 1){ // 기본 (일반파일모드)
		if(fileStr == NULL) fileStr = "data.txt";
		isThisFifo = 0;
	}
	else if((argc == 3) && (strcmp(argv[1], "-t") == 0)){
		if(strcmp(argv[2], "f") == 0){ // fifo 모드
			if(fileStr == NULL) fileStr = "data.fifo";
			isThisFifo = 1;

		}
		else if(strcmp(argv[2], "r") == 0){ // 일반 파일 모드
			if(fileStr == NULL) fileStr = "data.txt";
			isThisFifo = 0;
		}
		else{ // 잘못된 인자 입력시 오류 반환
			perror("Wrong Argument\n");
			exit(1);
		}
	}
	else if((argc == 3) && (strcmp(argv[1], "-f") == 0)){ // 사용자 파일명 지정 모드
		fileStr = argv[2]; 
		if(access(fileStr, F_OK) == -1){	
			isThisFifo = checkFileFifo(fileStr, ".");
		}
		else{
			isThisFifo = -1;
		}
	}
	else{ // 잘못된 인자 입력시 오류 반환
		perror("Wrong Argument\n");
		exit(1);
	}
	
	//디렉토리 안에 data.txt 파일이 있는지 확인
	if((dp = opendir(".")) == NULL){
		perror("opendir: source");
		exit(1);
	}
	while(notFoundDataTxt){
		while((dent = readdir(dp))){
			if(0 == strcmp(dent->d_name, fileStr)){
				notFoundDataTxt = false;
			}
		}
		rewinddir(dp);
	}
	closedir(dp);
	
	// fileStr이 가리키는 파일명을 가지는 파일 열기
	fd = open(fileStr, O_RDONLY);
	if(fd == -1){
		perror("Open file");
		exit(1);
	}
	
	// 연 파일이 어떤 종류의 파일인지 확인하여 설정된 모드에 맞는 종류의 파일이 아닐경우 오류를 반환
	lstat(fileStr, &fsbuf);
	changeTime = (int)fsbuf.st_mtime;
	kind = fsbuf.st_mode & S_IFMT;
	switch(isThisFifo){
		case 1:
			if(kind != S_IFIFO){
				perror("this is not fifo file!\n");
				exit(1);
			}
			break;
		case 0:
			if(kind != S_IFREG){
				perror("this is not Reg file\n");
				exit(1);
			}
			break;
		default:
			if((kind != S_IFIFO) && (kind != S_IFREG)){
				perror("this is not fifo or Reg file\n");
				exit(1);
			}
			else{
				if(kind == S_IFIFO){
					isThisFifo = 1;
				}
				else{
					isThisFifo = 0;
				}
			}
	}
	
	// 연 파일의 권한을 확인하여 group과 other에 읽기, 쓰기 권한이 부여되어 있을경우 오류반환
	PermCheck = ((S_IREAD | S_IWRITE) >> 3) | ((S_IREAD | S_IWRITE) >> 6);
	if((fsbuf.st_mode & PermCheck) != 0){
		perror("data.txt must be protected\n");
		exit(1);
	}
	
	// 일반 파일 모드에서 send 프로그램이 recv프로그램에 비해 나중에 열렸을 때의 오류 수정	
	if(isThisFifo == 0){
		saveSize = (int)fsbuf.st_size;
		while(changeTime == (int)fsbuf.st_mtime){
			stat(fileStr, &fsbuf);	
		}
		changeTime = (int)fsbuf.st_mtime;
		if(fsbuf.st_size != 0){
			lseek(fd, saveSize, SEEK_SET);
		}
	}
	
	// 파일을 이용한 통신을 위한 로직을 반복적으로 실행
	while(1){
		stat("data.txt", &fsbuf);
		if(((int)fsbuf.st_size == 0) && (isThisFifo == 0)){
			lseek(fd, 0, SEEK_SET);
		}
		n = read(fd, buf, 255);
		buf[n] = '\0';
		if(n == -1){
			perror("Read error");
		} else if(n == 0) continue;
		write(1, "Recv>> ", 7);
		write(1, buf, n);
		if(n == 1 && buf[0] == 'q'){
			write(1, "Terminate\n", 10);
			break;
		}
	}
	close(fd);
	return 0;
}


// 사용자 파일명 지정 모드에서 파일의 이름을 통해 파일의 종류를 파악하여 알맞은 숫자를 반환하는 함수
int checkFileFifo(char *str, char *splitPara){
	char *strUseCopy = (char *)malloc(strlen(str)+1);
	strcpy(strUseCopy, str);
	char *temp = (char *)malloc(strlen(str)+1);
	char *ptr2 = strtok(strUseCopy, splitPara);
	while(ptr2 != NULL){
		strcpy(temp, ptr2);
		ptr2 = strtok(NULL, splitPara);
	}
	if(strcmp(temp, "fifo") == 0){
		free(strUseCopy);
		free(temp);
		return 1;
	}
	else{
		free(strUseCopy);
		free(temp);
		return 0;
	}
}
